#include <iostream>
#include <unistd.h>
#include <fstream>
#include <vector>
#include <sys/time.h>
#include <ctime>
#include <chrono>
#include <pthread.h>

using std::chrono::system_clock;
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::cout;
using std::endl;
using std::ifstream;
using std::ofstream;
using std::vector;

#pragma pack(1)
#pragma once

typedef int LONG;
typedef unsigned short WORD;
typedef unsigned int DWORD;

typedef struct tagBITMAPFILEHEADER
{
  WORD bfType;
  DWORD bfSize;
  WORD bfReserved1;
  WORD bfReserved2;
  DWORD bfOffBits;
} BITMAPFILEHEADER, *PBITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER
{
  DWORD biSize;
  LONG biWidth;
  LONG biHeight;
  WORD biPlanes;
  WORD biBitCount;
  DWORD biCompression;
  DWORD biSizeImage;
  LONG biXPelsPerMeter;
  LONG biYPelsPerMeter;
  DWORD biClrUsed;
  DWORD biClrImportant;
} BITMAPINFOHEADER, *PBITMAPINFOHEADER;

int rows;
int cols;
vector<vector<vector<unsigned char>>> photo;

bool fillAndAllocate(char *&buffer, const char *fileName, int &rows, int &cols, int &bufferSize)
{
  std::ifstream file(fileName);

  if (file)
  {
    file.seekg(0, std::ios::end);
    std::streampos length = file.tellg();
    file.seekg(0, std::ios::beg);

    buffer = new char[length];
    file.read(&buffer[0], length);

    PBITMAPFILEHEADER file_header;
    PBITMAPINFOHEADER info_header;

    file_header = (PBITMAPFILEHEADER)(&buffer[0]);
    info_header = (PBITMAPINFOHEADER)(&buffer[0] + sizeof(BITMAPFILEHEADER));
    rows = info_header->biHeight;
    cols = info_header->biWidth;
    bufferSize = file_header->bfSize;
    return 1;
  }
  else
  {
    cout << "File" << fileName << " doesn't exist!" << endl;
    return 0;
  }
}




// void getPixlesFromBMP24(int end, int rows, int cols, char *fileReadBuffer)
// {
//   int count = 1;
//   int extra = cols % 4;
//   for (int i = 0; i < rows; i++)
//   {
//     count += extra;
//     for (int j = cols - 1; j >= 0; j--)
//       for (int k = 0; k < 3; k++)
//       {
//         switch (k)
//         {
//         case 0:
//           photo[0][i][j]=fileReadBuffer[end - count];
//           // fileReadBuffer[end - count] is the red value
//           break;
//         case 1:
//           photo[1][i][j]=fileReadBuffer[end - count];
//           // fileReadBuffer[end - count] is the green value
//           break;
//         case 2:
//           photo[2][i][j]=fileReadBuffer[end - count];
//           // fileReadBuffer[end - count] is the blue value
//           break;
//         // go to the next position in the buffer
//         }
//         count+=1;
//       }
//   }
// }




struct gpfb{
  int end;
  int count;
  char *fileReadBuffer;
  int i;
  gpfb(int _i,int e, int c,char *f){
    end=e;
    count=c;
    fileReadBuffer=f;
    i=_i;
  }
};

void* gpfbmphRow(void* input){
  gpfb* in=(gpfb*)input;
  int end =in->end;
  int count=in->count;
  char *fileReadBuffer=in->fileReadBuffer;
  int i=in->i;
  for (int j = cols - 1; j >= 0; j--)
      for (int k = 0; k < 3; k++)
      {
        switch (k)
        {
        case 0:
          photo[0][i][j]=fileReadBuffer[end - count];
          // fileReadBuffer[end - count] is the red value
          break;
        case 1:
          photo[1][i][j]=fileReadBuffer[end - count];
          // fileReadBuffer[end - count] is the green value
          break;
        case 2:
          photo[2][i][j]=fileReadBuffer[end - count];
          // fileReadBuffer[end - count] is the blue value
          break;
        // go to the next position in the buffer
        }
        count+=1;
      }
   pthread_exit(0);
}

void getPixlesFromBMP24(int end, int rows, int cols, char *fileReadBuffer)
{
  int count = 1;
  int extra = cols % 4;
  vector<pthread_t> threads(rows);
  gpfb* r;
  for (int i = 0; i < rows; i++)
  {
    count += extra;
    r=new gpfb(i,end,count,fileReadBuffer);
    pthread_create(&threads[i],NULL,gpfbmphRow,(void*)r);
    count+=3*cols;
  }
  for(int i=0;i<threads.size();i++){
    pthread_join(threads[i],NULL);
  }
  
}
/*.......................................................................*/








void print();

void writeOutBmp24(char *fileBuffer, const char *nameOfFileToCreate, int bufferSize)
{
  //print();
  //cout<<nameOfFileToCreate<<endl;
  std::ofstream write(nameOfFileToCreate);
  if (!write)
  {
    cout << "Failed to write " << nameOfFileToCreate << endl;
    return;
  }
  int count = 1;
  int extra = cols % 4;
  for (int i = 0; i < rows; i++)
  {
    count += extra;
    for (int j = cols - 1; j >= 0; j--)
      for (int k = 0; k < 3; k++)
      {
        switch (k)
        {
        case 0:
          fileBuffer[bufferSize - count]=photo[0][i][j];
          // write red value in fileBuffer[bufferSize - count]
          break;
        case 1:
          fileBuffer[bufferSize - count]=photo[1][i][j];
          // write green value in fileBuffer[bufferSize - count]
          break;
        case 2:
          fileBuffer[bufferSize - count]=photo[2][i][j];
          // write blue value in fileBuffer[bufferSize - count]
          break;
        // go to the next position in the buffer
        }
        count+=1;
      }
  }
  write.write(fileBuffer, bufferSize);
}

vector<vector<vector<unsigned char>>> newPhoto;

struct img
{
  int i;
  int j;
  int k;
  img(int _k,int _i, int _j){
    i=_i;
    j=_j;
    k=_k;
  }
};

int smooth(int k,int i, int j){
  int sum=0;
  for(int x=i-1;x<=i+1;x++){
    if(x<0||x>=rows)
      continue;
    for(int y=j-1;y<=j+1;y++){
      if(y<0||y>=cols)
        continue;
      sum+=photo[k][x][y];
    }
  }
  return sum/9;
}

struct smoothIn{
  int i;
  int s;
  int e;
  smoothIn(int _i,int _s,int _e){
    i=_i;
    s=_s;
    e=_e;
  }
};

void* smoothRow(void* input){
  smoothIn* in=(smoothIn*)input;
  int i=in->i;
  for(int j=in->s;j<in->e;j++){
      newPhoto[0][i][j]=smooth(0,i,j);
      newPhoto[1][i][j]=smooth(1,i,j);
      newPhoto[2][i][j]=smooth(2,i,j);
    }
   pthread_exit(0);
}

void smoothFilter(){
  newPhoto=photo;
  vector<pthread_t> threads(rows);
  smoothIn* r;
  for(int i=0;i<rows;i++){
    r=new smoothIn(i,0,cols);
    pthread_create(&threads[i],NULL,smoothRow,(void*)r);
  }
  for(int i=0;i<threads.size();i++){
    pthread_join(threads[i],NULL);
  }
  photo=newPhoto;
}



// void* smooth(void* input){
//   img* in=(img*)input;
//   int sum=0;
//   for(int x=(in->i-1);x<=(in->i+1);x++){
//     if(x<0||x>=rows)
//       continue;
//     for(int y=(in->j-1);y<=(in->j+1);y++){
//       if(y<0||y>=cols)
//         continue;
//       sum+=photo[in->k][x][y];
//     }
//   }
//    newPhoto[in->k][in->i][in->j]=sum/9;
//    pthread_exit(0);
// }

// void smoothFilter(){
//   newPhoto=photo;
//   vector<pthread_t> threads(cols*rows*3);
//   img* args;
//   pthread_t* pt;
//   int count=0;
//   for(int i=0;i<rows;i++){
//     for(int j=0;j<cols;j++){
//       for(int k=0;k<3;k++){
//         args=new img(k,i,j);
//         pthread_create(&threads[count],NULL,smooth,(void*)args);
//         count++;
//       }
//     }
//   }
//   for(int i=0;i<threads.size();i++){
//     pthread_join(threads[i],NULL);
//   }
//   photo=newPhoto;
// }

int sepia(int k,int i, int j){
  int red=photo[0][i][j],green=photo[1][i][j],blue=photo[2][i][j];
  int res;
  switch (k)
  {
  case 0:
    res=0.393*red+0.769*green+0.189*blue;
    if(res>255)
      return 255;
    return res;
    break;
  case 1:
    res=0.349*red+0.686*green+0.168*blue;
    if(res>255)
      return 255;
    return res;
    break;
  case 2:
    res=0.272*red+0.534*green+0.131*blue;
    if(res>255)
      return 255;
    return res; 
    break;
  }
}

void* sepialRow(void* input){
  int* in=(int*)input;
  int i=*in;
  for(int j=0;j<cols;j++){
      newPhoto[0][i][j]=sepia(0,i,j);
      newPhoto[1][i][j]=sepia(1,i,j);
      newPhoto[2][i][j]=sepia(2,i,j);
    }
   pthread_exit(0);
}

void sepiaFilter(){
 newPhoto=photo;
  vector<pthread_t> threads(rows);
  int* r;
  for(int i=0;i<rows;i++){
    r=new int(i);
    pthread_create(&threads[i],NULL,sepialRow,(void*)r);
  }
  for(int i=0;i<threads.size();i++){
    pthread_join(threads[i],NULL);
  }
  photo=newPhoto;
}

int answ[3];

void* getMean(void* in){
  int k=*(int*)in;
  int sum=0;
  int count=0;
  for(int i=0;i<rows;i++){
    for(int j=0;j<cols;j++){
      sum+=photo[k][i][j];
      count+=1;
    }
  }
  answ[k]=sum/count;
  pthread_exit(0);
}

void* meanRow(void* input){
  int* in=(int*)input;
  int i=*in;
  for(int j=0;j<cols;j++){
      newPhoto[0][i][j]= 0.4*newPhoto[0][i][j]+0.6*answ[0];
      newPhoto[1][i][j]= 0.4*newPhoto[1][i][j]+0.6*answ[1];
      newPhoto[2][i][j]= 0.4*newPhoto[2][i][j]+0.6*answ[2];
    }
   pthread_exit(0);
}

void meanFilter(){
  newPhoto=photo;
  vector<pthread_t> threads(rows);
  int* r;
  int mr,mg,mb;
  pthread_t t1,t2,t3;
  pthread_create(&t1,NULL,getMean,new int(0));
  pthread_create(&t2,NULL,getMean,new int(1));
  pthread_create(&t3,NULL,getMean,new int(2));
  pthread_join(t1,NULL);
  pthread_join(t2,NULL);
  pthread_join(t3,NULL);
  for(int i=0;i<rows;i++){
    r=new int(i);
    pthread_create(&threads[i],NULL,meanRow,(void*)r);
  }
  for(int i=0;i<threads.size();i++){
    pthread_join(threads[i],NULL);
  }
  photo=newPhoto;
}

void xFilter(){
  vector<vector<vector<unsigned char>>> newPhoto=photo;
  for(int i=0;i<rows;i++){
    for(int k=0;k<3;k++){
      if(i>0){
        newPhoto[k][i-1][i]=255;
        newPhoto[k][rows-i][i]=255;
      }
      newPhoto[k][i][i]=255;
      newPhoto[k][rows-i-1][i]=255;
      if((i+1)<rows){
        newPhoto[k][i+1][i]=255;
        newPhoto[k][rows-i-2][i]=255;
      }
    }
  }
  photo=newPhoto;
}

void blackFilter(){
  vector<vector<vector<unsigned char>>> newPhoto=photo;
  for(int i=0;i<rows;i++){
    for(int j=0;j<cols;j++){
      newPhoto[0][i][j]=0;
      newPhoto[1][i][j]=0;
      newPhoto[2][i][j]=0;
    }
  }
  photo=newPhoto;
}

void print(){
  for(int i=0;i<rows;i++){
    for(int j=0;j<cols;j++){
      cout<<"(";
      cout<<photo[0][i][j]<<", ";
      cout<<photo[1][i][j]<<", ";
      cout<<photo[2][i][j]<<") ";
    }
    cout<<endl;
  }
}

int main(int argc, char *argv[])
{
  char *fileBuffer;
  int bufferSize;
  char *fileName = argv[1];
  char outFile[]="out.bmp";
  //time_t t1,t2,t3,t4,t5,t6,t7;
  auto t0=duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
  if (!fillAndAllocate(fileBuffer, fileName, rows, cols, bufferSize))
  {
    cout << "File read error" << endl;
    return 1;
  }
  vector<vector<unsigned char>> page =vector<vector<unsigned char>>(rows,vector<unsigned char>(cols,0));
  photo.push_back(page);
  photo.push_back(page);
  photo.push_back(page);
  //auto t1=duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
  getPixlesFromBMP24(bufferSize,rows,cols,fileBuffer);
  //auto t2=duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
  smoothFilter();
  //auto t3=duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
  sepiaFilter();
  //auto t4=duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
  meanFilter();
  //auto t5=duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
  xFilter();
  //auto t6=duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
  writeOutBmp24(fileBuffer,outFile,bufferSize);
  auto t7=duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
  //cout<<t1-t0<<":"<<t2-t1<<":"<<t3-t2<<":"<<t4-t3<<":"<<t5-t4<<":"<<t6-t5<<":"<<t7-t6<<":"<<t7-t0;
  cout<<t7-t0<<endl;
  return 0;
}