#include "bufferPoolManager.h"
#include <cstdio>
#include <random>
#include <string>
#include<iostream>


int main() {
  const std::string db_name = "test1.out";
  const size_t buffer_pool_size = 100;

  auto *bpm = new BufferPoolManager(buffer_pool_size);

  pageId_t page_id_temp;
  /*auto *page0 = bpm->newPage(db_name,page_id_temp);

  std::cout<<page_id_temp<<std::endl;

  snprintf(page0->getContent(), PAGESIZE, "Hello");
  if(strcmp(page0->getContent(),"Hello")==0){
    std::cout<<"strcmp(page0->getContent(),\"Hello\")==0"<<std::endl;
  }
  else{
     std::cout<<"strcmp(page0->getContent(),\"Hello\")!=0"<<std::endl;
  }*/

  /*for (size_t i = 0; i < buffer_pool_size; ++i) {
   auto a=bpm->newPage(db_name,page_id_temp);
  }*/
   for (size_t i = 0; i < buffer_pool_size; ++i) {
   auto a=bpm->fetchPage((static_cast<pageId_t>(i)),db_name);
   if(a==nullptr)
   std::cout<<":nullptr"<<std::endl;
   else
   std::cout<<a->getPageId()<<":non_nullptr"<<std::endl;
  }

  /*page0 = bpm->fetchPage(0,db_name);
  if(strcmp(page0->getContent(), "Hello")==0){
    std::cout<<"strcmp(page0->getContent(), \"Hello\")==0"<<std::endl;
  }
  else{
    std::cout<<"strcmp(page0->getContent(), \"Hello\")!=0"<<std::endl;
  }*/
  int s=bpm->getFileLastPageId(db_name);
  std::cout<<"getFileLastPageId"<<s<<std::endl;
 //bpm->deleteFileAndPages(db_name);

 /* for (size_t i = 0; i < buffer_pool_size; ++i) {
   auto a=bpm->fetchPage((static_cast<pageId_t>(i)),db_name);
   if(a==nullptr)
   std::cout<<"nullptr"<<std::endl;
   else
   std::cout<<a->getPageId()<<":non_nullptr"<<std::endl;
  }*/
  delete bpm;
}

