// cat1.d
// works
import windows;

extern (Windows) HANDLE GetStdHandle(DWORD nStdHandle);
const DWORD STD_INPUT_HANDLE=(DWORD)(0xfffffff6),
            STD_OUTPUT_HANDLE=(DWORD)(0xfffffff5),
            STD_ERROR_HANDLE=(DWORD)(0xfffffff4);

int main(char[][] args) {
  HANDLE hstdin=GetStdHandle(STD_INPUT_HANDLE);
  HANDLE hstdout=GetStdHandle(STD_OUTPUT_HANDLE);
  DWORD got;
  char c;
  while (ReadFile(hstdin,&c,1,&got,null) && got) {
    WriteFile(hstdout,&c,1,&got,null);
  }
  return 0;
}
