#include <inttypes.h>
#include <poll.h>
#include <sstream>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <time.h>

#include "smb2.h"
#include "util.h"

// 将字符串按空格切割
std::vector<std::string> split_by_space(const std::string &input) {
  std::istringstream iss(input);
  std::vector<std::string> tokens;
  std::string token;
  while (iss >> token) {
    tokens.push_back(token);
  }
  return tokens;
}

struct ex_params {
  smb2_url url;
  std::string password = "1";

  ex_params() {
    url.domain = "";
    url.user = "wangbin";
    url.server = "172.27.93.12";
    url.share = "smb_shared";
    url.path = "";
  }
};

std::string err;
Smb2ContextPtr smb2;

void usage(void) {
  fprintf(stderr,
          "Usage:\n"
          "    l [path]   show files in optional path (default is root)\n"
          "    q          exit program\n"
          "    h          print usage\n"
          "\n");
};

void show_dir(std::string query_path) {
  printf("query: [%s]\n", query_path.c_str());
  std::string pattern = "*";
  smb2dir *dir = smb2->smb2_querydir(query_path, pattern, err);
  if (dir == NULL) {
    printf("smb2_opendir failed. %s\n", err.c_str());
    exit(10);
  }

  for (smb2dirent ent : dir->entries) {
    if (ent.name == "." || ent.name == "..")
      continue;
    std::string type;

    switch (ent.st.smb2_type) {
    case SMB2_TYPE_FILE:
      type = std::string("FILE");
      break;
    case SMB2_TYPE_DIRECTORY:
      type = std::string("DIRECTORY");
      break;
    default:
      type = std::string("unknown");
      break;
    }

    uint64_t filetime = ent.st.smb2_mtime;
    time_t t = 0;
    if (filetime > 116444736000000000ULL)
      t = (filetime - 116444736000000000ULL) / 10000000;

    char time_buf[64];
    strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", localtime(&t));

    printf("    %-20s %-9s %15" PRIu64 " %s\n", ent.name.c_str(), type.c_str(),
           ent.st.smb2_size, time_buf);
  }

  smb2->smb2_closedir(dir);
}

int main(int argc, char *argv[]) {
  smb2 = Smb2Context::create();

  ex_params params;
  smb2->smb2SetUser(params.url.user);
  smb2->smb2SetPassword(params.password);
  smb2->smb2SetDomain(params.url.domain);
  smb2->smb2SetAuthMode(SMB2_SEC_NTLMSSP);
  smb2->smb2SetSecurityMode(0);

  if (smb2->smb2_connect_share(params.url.server, params.url.share,
                               params.url.user, err) != 0) {
    printf("smb2_connect_share failed. %s\n", err.c_str());
    exit(10);
  }

  usage();
  while (true) {

    std::string cmd;

    // std::cin 从标准输入读取一个“单词”，遇到空格、Tab 或换行符
    // std::cin >> cmd;
    // 读取整行
    std::getline(std::cin, cmd);

    auto cmd_params = split_by_space(cmd);
    std::cout << "[input:] " << cmd << std::endl;
    if (cmd_params[0] == "l") {
      // 路径必须已存在的dir
      std::string path = "";
      if (cmd_params.size() > 1) {
        path = cmd_params[1];
      }
      show_dir(path);
    } else if (cmd == "q") {
      break;
    } else if (cmd == "h") {
      usage();
    } else {
      std::cout << "param failed, continue\n";
    }
    printf("\n");
  }

  smb2->smb2_disconnect_share();

  return 0;
}
