#include "helpers/process/process.h"

#include <poll.h>
#include <sys/wait.h>
#include <unistd.h>

#include <array>

namespace {

constexpr size_t kReadChunkSize = 4096;

int exitStatus(pid_t child) {
  int status = 0;
  waitpid(child, &status, 0);
  return WIFEXITED(status) ? WEXITSTATUS(status) : kSignalStatusBase + WTERMSIG(status);
}

std::vector<char*> buildArgv(const fs::path& tool, const std::vector<std::string>& args) {
  std::vector<char*> argv{const_cast<char*>(tool.c_str())};
  for (const auto& arg : args) {
    argv.push_back(const_cast<char*>(arg.c_str()));
  }
  argv.push_back(nullptr);
  return argv;
}

[[noreturn]] void execute(const fs::path& tool, std::vector<char*>& argv) {
  execv(tool.c_str(), argv.data());
  _exit(kExecFailedStatus);
}

void forward(int fileDescriptor, const char* data, size_t size) {
  if (write(fileDescriptor, data, size) < 0) {
    return;
  }
}

void drain(int outFd, int errFd, RunResult& result, Echo echo) {
  std::array<pollfd, 2> fds{pollfd{.fd = outFd, .events = POLLIN, .revents = 0},
                            pollfd{.fd = errFd, .events = POLLIN, .revents = 0}};
  std::array<std::string*, 2> sinks{&result.out, &result.err};
  for (int openStreams = 2; openStreams > 0;) {
    poll(fds.data(), fds.size(), -1);
    for (size_t stream = 0; stream < fds.size(); ++stream) {
      pollfd& watched = fds[stream];
      if (watched.fd < 0 || (watched.revents & (POLLIN | POLLHUP)) == 0) {
        continue;
      }
      std::array<char, kReadChunkSize> buffer{};
      ssize_t count = read(watched.fd, buffer.data(), buffer.size());
      if (count > 0) {
        sinks[stream]->append(buffer.data(), static_cast<size_t>(count));
        if (echo == Echo::On) {
          forward(static_cast<int>(stream) + 1, buffer.data(), static_cast<size_t>(count));
        }
      } else {
        close(watched.fd);
        watched.fd = -1;
        --openStreams;
      }
    }
  }
}

}  // namespace

int runInherited(const fs::path& tool, const std::vector<std::string>& args) {
  std::vector<char*> argv = buildArgv(tool, args);
  pid_t child = fork();
  if (child < 0) {
    return kExecFailedStatus;
  }
  if (child == 0) {
    execute(tool, argv);
  }
  return exitStatus(child);
}

RunResult runCaptured(const fs::path& tool, const std::vector<std::string>& args, Echo echo,
                      const fs::path& workingDirectory) {
  std::vector<char*> argv = buildArgv(tool, args);
  std::array<int, 2> outPipe{};
  std::array<int, 2> errPipe{};
  if (pipe(outPipe.data()) != 0 || pipe(errPipe.data()) != 0) {
    return execFailed();
  }
  pid_t child = fork();
  if (child < 0) {
    return execFailed();
  }
  if (child == 0) {
    if (!workingDirectory.empty() && chdir(workingDirectory.c_str()) != 0) {
      _exit(kExecFailedStatus);
    }
    dup2(outPipe[1], STDOUT_FILENO);
    dup2(errPipe[1], STDERR_FILENO);
    close(outPipe[0]);
    close(outPipe[1]);
    close(errPipe[0]);
    close(errPipe[1]);
    execute(tool, argv);
  }
  close(outPipe[1]);
  close(errPipe[1]);
  RunResult result;
  drain(outPipe[0], errPipe[0], result, echo);
  result.exitCode = exitStatus(child);
  return result;
}
