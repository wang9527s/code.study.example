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
          "    push <from> <to>  push file to server\n"
          "    info <pathname>   printf file infos\n"
          "    rm <pathname> delete file or dir\n"
          "    mkdir <path> \n"
          "    sl         show remote shared list\n"
          "    sec <pathname> get_security\n"
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
  if (smb2->smb2_list_shares(params.url.server, params.url.user,
                             2, /*query share info type*/
                             shares, err) < 0) {
    printf("failed to get share list Error : %s\n", err.c_str());
    return;
  }

  for (smb2_shareinfo entry : shares.sharelist) {
    if (shares.share_info_type == 1) {
      printf("    [name]: %-20s [type]: %-11x\n", entry.name.c_str(),
             entry.share_type);
    } else if (shares.share_info_type == 2) {
      printf("    [name]: %-20s [type]:%-11x [path]:%-100s\n",
             entry.name.c_str(), entry.share_type, entry.path.c_str());
    }
  }
}

void show_info(std::string path) {
  time_t t;
  printf("\n ---------- \n stat_all: \n");
  {
    struct smb2_file_info_all fs;
    if (smb2->smb2_getinfo_all(path, &fs, err) != 0) {
      printf("failed waiting for a reply. %s\n", err.c_str());
      exit(10);
    }

    /* Print the file_all_info structure */
    printf("Attributes: ");
    if (fs.file_attributes & SMB2_FILE_ATTRIBUTE_READONLY) {
      printf("READONLY ");
    }
    if (fs.file_attributes & SMB2_FILE_ATTRIBUTE_HIDDEN) {
      printf("HIDDEN ");
    }
    if (fs.file_attributes & SMB2_FILE_ATTRIBUTE_SYSTEM) {
      printf("SYSTEM ");
    }
    if (fs.file_attributes & SMB2_FILE_ATTRIBUTE_DIRECTORY) {
      printf("DIRECTORY ");
    }
    if (fs.file_attributes & SMB2_FILE_ATTRIBUTE_ARCHIVE) {
      printf("ARCHIVE ");
    }
    if (fs.file_attributes & SMB2_FILE_ATTRIBUTE_NORMAL) {
      printf("NORMAL ");
    }
    if (fs.file_attributes & SMB2_FILE_ATTRIBUTE_TEMPORARY) {
      printf("TMP ");
    }
    if (fs.file_attributes & SMB2_FILE_ATTRIBUTE_SPARSE_FILE) {
      printf("SPARSE ");
    }
    if (fs.file_attributes & SMB2_FILE_ATTRIBUTE_REPARSE_POINT) {
      printf("REPARSE ");
    }
    if (fs.file_attributes & SMB2_FILE_ATTRIBUTE_COMPRESSED) {
      printf("COMPRESSED ");
    }
    if (fs.file_attributes & SMB2_FILE_ATTRIBUTE_OFFLINE) {
      printf("OFFLINE ");
    }
    if (fs.file_attributes & SMB2_FILE_ATTRIBUTE_NOT_CONTENT_INDEXED) {
      printf("NOT_CONTENT_INDEXED ");
    }
    if (fs.file_attributes & SMB2_FILE_ATTRIBUTE_ENCRYPTED) {
      printf("ENCRYPTED ");
    }
    if (fs.file_attributes & SMB2_FILE_ATTRIBUTE_INTEGRITY_STREAM) {
      printf("INTEGRITY_STREAM ");
    }
    if (fs.file_attributes & SMB2_FILE_ATTRIBUTE_NO_SCRUB_DATA) {
      printf("NO_SCRUB_DATA ");
    }
    printf("\n");

    t = SMBTimeToUTime(fs.smb2_crtime);
    printf("Creation Time:    %s", asctime(localtime(&t)));
    t = SMBTimeToUTime(fs.smb2_atime);
    printf("Last Access Time: %s", asctime(localtime(&t)));
    t = SMBTimeToUTime(fs.smb2_mtime);
    printf("Last Write Time:  %s", asctime(localtime(&t)));
    t = SMBTimeToUTime(fs.smb2_ctime);
    printf("Change Time:      %s", asctime(localtime(&t)));

    printf("Allocation Size: %" PRIu64 "\n", fs.allocation_size);
    printf("End Of File:     %" PRIu64 "\n", fs.end_of_file);
    printf("Number Of Links: %d\n", fs.smb2_nlink);
    printf("Delete Pending:  %s\n", fs.delete_pending ? "YES" : "NO");
    printf("Directory:       %s\n", fs.directory ? "YES" : "NO");

    printf("Inode Number: 0x%016" PRIx64 "\n", fs.smb2_ino);
    printf("EA Size : %d\n", fs.ea_size);

    printf("Access Flags: ");
    if (fs.directory) {
      if (fs.access_flags & SMB2_FILE_LIST_DIRECTORY) {
        printf("LIST_DIRECTORY ");
      }
      if (fs.access_flags & SMB2_FILE_ADD_FILE) {
        printf("ADD_FILE ");
      }
      if (fs.access_flags & SMB2_FILE_ADD_SUBDIRECTORY) {
        printf("ADD_SUBDIRECTORY ");
      }
      if (fs.access_flags & SMB2_FILE_TRAVERSE) {
        printf("TRAVERSE ");
      }
    } else {
      if (fs.access_flags & SMB2_FILE_READ_DATA) {
        printf("READ_DATA ");
      }
      if (fs.access_flags & SMB2_FILE_WRITE_DATA) {
        printf("WRITE_DATA ");
      }
      if (fs.access_flags & SMB2_FILE_APPEND_DATA) {
        printf("APPEND_DATA ");
      }
      if (fs.access_flags & SMB2_FILE_EXECUTE) {
        printf("FILE_EXECUTE ");
      }
    }
    if (fs.access_flags & SMB2_FILE_READ_EA) {
      printf("READ_EA ");
    }
    if (fs.access_flags & SMB2_FILE_WRITE_EA) {
      printf("WRITE_EA ");
    }
    if (fs.access_flags & SMB2_FILE_READ_ATTRIBUTES) {
      printf("READ_ATTRIBUTES ");
    }
    if (fs.access_flags & SMB2_FILE_WRITE_ATTRIBUTES) {
      printf("WRITE_ATTRIBUTES ");
    }
    if (fs.access_flags & SMB2_FILE_DELETE_CHILD) {
      printf("DELETE_CHILD ");
    }
    if (fs.access_flags & SMB2_DELETE) {
      printf("DELETE ");
    }
    if (fs.access_flags & SMB2_READ_CONTROL) {
      printf("READ_CONTROL ");
    }
    if (fs.access_flags & SMB2_WRITE_DACL) {
      printf("WRITE_DACL ");
    }
    if (fs.access_flags & SMB2_WRITE_OWNER) {
      printf("WRITE_OWNER ");
    }
    if (fs.access_flags & SMB2_SYNCHRONIZE) {
      printf("SYNCHRONIZE ");
    }
    if (fs.access_flags & SMB2_ACCESS_SYSTEM_SECURITY) {
      printf("ACCESS_SYSTEM_SECURITY ");
    }
    if (fs.access_flags & SMB2_MAXIMUM_ALLOWED) {
      printf("MAXIMUM_ALLOWED ");
    }
    if (fs.access_flags & SMB2_GENERIC_ALL) {
      printf("GENERIC_ALL ");
    }
    if (fs.access_flags & SMB2_GENERIC_EXECUTE) {
      printf("GENERIC_EXECUTE ");
    }
    if (fs.access_flags & SMB2_GENERIC_WRITE) {
      printf("GENERIC_WRITE ");
    }
    if (fs.access_flags & SMB2_GENERIC_READ) {
      printf("GENERIC_READ ");
    }
    printf("\n");

    printf("Mode: ");
    if (fs.access_flags & SMB2_FILE_WRITE_THROUGH) {
      printf("WRITE_THROUGH ");
    }
    if (fs.access_flags & SMB2_FILE_SEQUENTIAL_ONLY) {
      printf("SEQUENTIAL_ONLY ");
    }
    if (fs.access_flags & SMB2_FILE_NO_INTERMEDIATE_BUFFERING) {
      printf("NO_INTERMEDIATE_BUFFERING ");
    }
    if (fs.access_flags & SMB2_FILE_SYNCHRONOUS_IO_ALERT) {
      printf("SYNCHRONOUS_IO_ALERT ");
    }
    if (fs.access_flags & SMB2_FILE_SYNCHRONOUS_IO_NONALERT) {
      printf("SYNCHRONOUS_IO_NONALERT ");
    }
    if (fs.access_flags & SMB2_FILE_DELETE_ON_CLOSE) {
      printf("DELETE_ON_CLOSE ");
    }
    printf("\n");
  }
  printf("\n ---------- \n stat: \n");
  {
    struct smb2_stat_64 st;
    if (smb2->smb2_stat(path, &st, err) < 0) {
      printf("smb2_stat failed. %s\n", err.c_str());
      return;
    }

    switch (st.smb2_type) {
    case SMB2_TYPE_FILE:
      printf("Type:FILE\n");
      break;
    case SMB2_TYPE_DIRECTORY:
      printf("Type:DIRECTORY\n");
      break;
    default:
      printf("Type:unknown\n");
      break;
    }
    printf("Size:%" PRIu64 "\n", st.smb2_size);
    printf("Inode:0x%" PRIx64 "\n", st.smb2_ino);
    printf("Links:%" PRIu32 "\n", st.smb2_nlink);
    t = SMBTimeToUTime(st.smb2_atime);
    printf("Atime:%s", asctime(localtime(&t)));
    t = SMBTimeToUTime(st.smb2_mtime);
    printf("Mtime:%s", asctime(localtime(&t)));
    t = SMBTimeToUTime(st.smb2_ctime);
    printf("Ctime:%s", asctime(localtime(&t)));
  }
  printf("\n ---------- \n vfs: \n");
  {
    struct smb2_statvfs vfs;
    if (smb2->smb2_statvfs(path, &vfs, err) < 0) {
      printf("smb2_statvfs failed. %s\n", err.c_str());
      return;
    }
    printf("Blocksize:%d\n", vfs.f_bsize);
    printf("Blocks:%" PRIu64 "\n", vfs.f_blocks);
    printf("Free:%" PRIu64 "\n", vfs.f_bfree);
    printf("Avail:%" PRIu64 "\n", vfs.f_bavail);
  }
}

void push(std::string from, std::string to) {
  smb2fh *fh;
  int count;
  int fd;
  uint8_t buf[256 * 1024];

  fd = open(from.c_str(), O_RDONLY);
  if (fd == -1) {
    printf("Failed to open local file %s (%s)\n", from.c_str(),
           strerror(errno));
    return;
  }

  fh = smb2->smb2_open(to, O_WRONLY | O_CREAT, err);
  if (fh == NULL) {
    printf("smb2_open failed. %s\n", err.c_str());
    return;
  }

  // TODO 原本的demo，这里有问题，smb2_write不成功。调半天了
  /*
  while ((count = read(fd, buf, 1024)) > 0)
  {
    smb2->smb2_write(fh, buf, count, err);
  };
  */
  int offset = 0;
  while ((count = read(fd, buf, 1024)) > 0) {
    if (smb2->smb2_pwrite(fh, buf, count, offset, err) !=
        SMB2_STATUS_SUCCESS) {
      fprintf(
          stderr,
          "smb2_pwrite failed at offset %lld (%zu bytes written so far): %s\n",
          static_cast<long long>(offset), static_cast<size_t>(offset),
          err.empty() ? "Unknown error" : err.c_str());
      break;
    }
    offset += count;
  }
  close(fd);
  smb2->smb2_close(fh, err);
}

void get_security(std::string pathname) {
  uint8_t *securityBuf = NULL;
  uint32_t secLen = 0;
  if (smb2->smb2_get_security(pathname, &securityBuf, &secLen, err) != 0) {
    printf("Failed to get security descriptor - %s\n", err.c_str());
    return;
  }

  struct smb2_security_descriptor *sd = nullptr;
  smb2DecodeSecurityDescriptor(&sd, securityBuf, secLen, err);

  printSecurityDescriptor(sd);
  smb2FreeSecurityDescriptor(sd);
  free(securityBuf);
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
    std::getline(std::cin, cmd);
    // cmd = "push /home/wangbin/Readme.md Readme";

    auto cmd_params = split_by_space(cmd);
    std::cout << "do:  " << cmd << std::endl;
    if (cmd_params[0] == "l") {
      // 路径必须已存在的dir
      std::string path = "";
      if (cmd_params.size() > 1) {
        path = cmd_params[1];
      }
      show_dir(path);
    } else if (cmd_params[0] == "push") {
      if (cmd_params.size() < 3) {
        std::cout << "failed, please input path or name\n";
        continue;
      }
      push(cmd_params[1], cmd_params[2]);
    } else if (cmd_params[0] == "info") {
      if (cmd_params.size() < 2) {
        std::cout << "failed, please input path or name\n";
        continue;
      }
      std::string path = cmd_params[1];
      show_info(path);
    } else if (cmd_params[0] == "rm") {
      if (cmd_params.size() < 2) {
        std::cout << "failed, param err\n";
        continue;
      }
      if (smb2->smb2_unlink(cmd_params[1], err) != SMB2_STATUS_SUCCESS) {
        std::cout << "failed " << err << std::endl;
      }
    } else if (cmd_params[0] == "sec") {
      if (cmd_params.size() < 2) {
        std::cout << "failed, param err\n";
        continue;
      }
      get_security(cmd_params[1]);
    } else if (cmd_params[0] == "mkdir") {
      if (cmd_params.size() < 2) {
        std::cout << "failed, param err\n";
        continue;
      }
      if (smb2->smb2_mkdir(cmd_params[1], err) != SMB2_STATUS_SUCCESS) {
        std::cout << "failed " << err << std::endl;
      }
    } else if (cmd_params[0] == "cat") {
      if (cmd_params.size() < 2) {
        std::cout << "failed, please input filename\n";
        continue;
      }
      std::string pathname = cmd_params[1];
      prinf_file(pathname);
    } else if (cmd == "sl") {
      smb2->smb2_disconnect_share();
      /* 概率出现 failed to get share list Error : smb2_list_shares:
         IOCTL:DCE_OP_SHARE_ENUM Failed : smb2_ioctl: receivePdus: No matching
         PDU found
         应该是前面smb2_connect_share的问题
      */
      print_sharelist();
    } else if (cmd == "q") {
      break;
    } else if (cmd == "h") {
      usage();
    } else {
      std::cout << "param failed, continue\n";
    }
    printf("\n");

  } while (true);

  smb2->smb2_disconnect_share();

  return 0;
}
