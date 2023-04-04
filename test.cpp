//
// Created by 一杯 空气U on 2023/3/31.
//
#include "skiplist_zeefan.h"
//#include "skiplist.h"

int main()
{
  //测试链表

  // 构造跳表
  SkipList<int, int> skiplist(3);

  //增
  skiplist.insert_element(3, 3);
  skiplist.insert_element(6, 6);
  skiplist.insert_element(1, 1);
  skiplist.insert_element(7, 7);
  skiplist.insert_element(9, 9);
  skiplist.insert_element(4, 4);
  skiplist.insert_element(2, 2);
  skiplist.insert_element(5, 5);
  skiplist.insert_element(8, 8);

  //删
//  skiplist.delete_element(9);

  //查
  skiplist.search_element(8);

  //

  //展示
  skiplist.display_list();

  return 0;
}

