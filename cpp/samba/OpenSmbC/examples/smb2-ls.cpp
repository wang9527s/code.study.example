#include <inttypes.h>
#include <poll.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <string>

#include "smb2.h"
#include "util.h"

int usage(void)
{
        fprintf(stderr, "Usage:\n"
                "smb2-ls-sync <smb2-url>\n\n"
                "URL format: "
                "smb://[<domain;][<username>@]<host>/<share>/<path>\n");
        exit(1);
}

int main(int argc, char *argv[])
{
  std::string err;
  Smb2ContextPtr smb2;
  smb2 = Smb2Context::create();
  if (smb2 == NULL) {
    fprintf(stderr, "Failed to init context\n");
    exit(0);
  }

  // ... emm 可能存在版本兼容问题，之前版本参数解析的不太对，一直跑不通 
  //      user解析成了 wangbin:1   所以一直跑不通
  std::string server = "172.27.93.12";
  std::string share = "smb_shared";
  std::string user = "wangbin";   
  std::string password = "1";
  std::string domain = "";  // 如果没有域名就空着

  smb2->smb2SetUser(user);
  smb2->smb2SetPassword(password);
  smb2->smb2SetDomain(domain);
  smb2->smb2SetAuthMode(SMB2_SEC_NTLMSSP);
  smb2->smb2SetSecurityMode(0);

  if (smb2->smb2_connect_share(server, share, user, err) != 0) {
      printf("smb2_connect_share failed. %s\n", err.c_str());
      exit(10);
  }

  std::string pattern = "*";
  std::string query_path = "john.dir";
  smb2dir *dir = smb2->smb2_querydir(query_path, pattern, err);
  if (dir == NULL) {
    printf("smb2_opendir failed. %s\n", err.c_str());
    exit(10);
  }

  for (smb2dirent ent : dir->entries)
  {
    std::string type;
    time_t t;

    switch (ent.st.smb2_type)
    {
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
    t = SMBTimeToUTime(ent.st.smb2_mtime);
    printf("%-20s %-9s %15"PRIu64" %s\n", ent.name.c_str(), type.c_str(), ent.st.smb2_size, asctime(localtime(&t)));
  }

  smb2->smb2_closedir(dir);

  std::string path = "a.txt";
  uint8_t *buf = NULL; uint32_t buf_len = 0;
  smb2->smb2_get_security(path, &buf, &buf_len, err);
  smb2->smb2_disconnect_share();

  return 0;
}
