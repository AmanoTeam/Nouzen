#define FMODE_USER_EXEC (0x00000001)
#define FMODE_USER_WRITE (0x00000002)
#define FMODE_USER_READ (0x00000004)
#define FMODE_GROUP_EXEC (0x00000010)
#define FMODE_GROUP_WRITE (0x00000020)
#define FMODE_GROUP_READ (0x00000040)
#define FMODE_OTHERS_EXEC (0x00000080)
#define FMODE_OTHERS_WRITE (0x00000100)
#define FMODE_OTHERS_READ (0x00000200)

int get_file_permissions(const char* const path);
int set_file_writable(const char* const path);
