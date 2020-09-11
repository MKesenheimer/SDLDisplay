#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

using namespace std;

static int last_position=0;
// read file untill new line
// save position

int find_new_text(ifstream &infile) {

   infile.seekg(0,ios::end);
   int filesize = infile.tellg();

   // check if the new file started
   if(filesize < last_position){
      last_position=0;
   }  
   // read file from last position  untill new line is found 

   for(int n=last_position;n<filesize;n++) {

      infile.seekg( last_position,ios::beg);
      char  test[256]; 
      infile.getline(test, 256);
      last_position = infile.tellg();
      cout << "Char: "  << test <<"Last position " << last_position<<  endl;
      // end of file 
      if(filesize == last_position){
        return filesize;
      } 

  }

  return 0;
}


int main() {

  for(;;) {
    std::ifstream infile("stream.bin");
    int current_position = find_new_text(infile);
    //sleep(1);
  } 

} 
