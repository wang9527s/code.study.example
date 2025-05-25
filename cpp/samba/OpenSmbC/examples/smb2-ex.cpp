#include <fcntl.h>
#include <inttypes.h>
#include <poll.h>
#include <sstream>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <time.h>
#include <unistd.h>

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
ex_params params;

void usage(void) {
  fprintf(stderr,
          "Usage:\n"
          "    l [path]   show files in optional path (default is root)\n"
          "    cat <pathname>   printf files\n"
          "    sl         show remote shared list\n"
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

void prinf_file(std::string path) {
  smb2fh *fh = smb2->smb2_open(path, O_RDONLY, err);
  if (fh == NULL) {
    printf("smb2_open failed. %s\n", err.c_str());
    return;
  }
  uint8_t buf[256 * 1024];
  uint32_t pos = 0;

  uint32_t status = 0;
  while ((status = smb2->smb2_pread(fh, buf, 1024, pos, err)) !=
         SMB2_STATUS_END_OF_FILE) {
    write(0, buf, fh->byte_count);
    pos += fh->byte_count;
  }
  // EOF might have returned some data
  if (fh->byte_count) {
    write(0, buf, fh->byte_count);
    pos += fh->byte_count;
  }

  smb2->smb2_close(fh, err);
}

void print_sharelist() {
  smb2_shares shares;
  if (smb2->smb2_list_shares(params.url.server,
                             params.url.user,
                             2, /*query share info type*/
                             shares, err) < 0)
  {
    printf("failed to get share list Error : %s\n", err.c_str());
    return ;
  }

  for(smb2_shareinfo entry : shares.sharelist)
  {
    if (shares.share_info_type == 1) {
      printf("    [name]: %-20s [type]: %-11x\n", entry.name.c_str(), entry.share_type);
    }
    else if (shares.share_info_type == 2){
      printf("    [name]: %-20s [type]:%-11x [path]:%-100s\n", entry.name.c_str(), entry.share_type, entry.path.c_str());
    }
  }
}

int main(int argc, char *argv[]) {
  smb2 = Smb2Context::create();

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
  do {
    std::string cmd = "";

    // std::cin 从标准输入读取一个“单词”，遇到空格、Tab 或换行符
    // std::cin >> cmd;
    // 读取整行
    // std::getline(std::cin, cmd);
    cmd = "sl";

    auto cmd_params = split_by_space(cmd);
    std::cout << "do:  " << cmd << std::endl;
    if (cmd_params[0] == "l") {
      // 路径必须已存在的dir
      std::string path = "";
      if (cmd_params.size() > 1) {
        path = cmd_params[1];
      }
      show_dir(path);
    } else if (cmd_params[0] == "cat") {
      if (cmd_params.size() < 2) {
        std::cout << "failed, please input filename\n";
        continue;
      }
      std::string pathname = cmd_params[1];
      prinf_file(pathname);
    } else if (cmd == "sl") {
      smb2->smb2_disconnect_share();
      // 概率出现 failed to get share list Error : smb2_list_shares: IOCTL: DCE_OP_SHARE_ENUM Failed : smb2_ioctl: receivePdus: No matching PDU found
      // 应该是前面smb2_connect_share的问题
      print_sharelist();
    } else if (cmd == "q") {
      break;
    } else if (cmd == "h") {
      usage();
    } else {
      std::cout << "param failed, continue\n";
    }
    printf("\n");
  } while (false);

  smb2->smb2_disconnect_share();

  return 0;
}
