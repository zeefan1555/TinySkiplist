
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <mutex>
#include <fstream>
using  namespace  std;

std::mutex mtx;     // mutex for critical section
std::string delimiter = ":";


template<typename K, typename V>
class Node{
public:
  Node(){};
  Node(K k, V v, int);
  ~Node();
  K get_Key() const; // K return key
  V get_value() const; //~

  void set_value(V);

  Node<K, V> **forward;

  int node_level;

private:
  K key;
  V value;
};

template<typename K, typename V>
//初始化
Node<K,V>::Node(const K k , const V v, int level){ //const 保证传递的参数不被改变
  this->key = k;
  this->value = v;
  this->node_level = level;
  this->forward = new Node<K, V>* [level + 1];
  memset(this->forward, 0, sizeof(Node<K, V>)*(level + 1));
};

template<typename K, typename V>
Node<K, V>::~Node() {
  delete[] forward; //删除new 出来 forward数组
};

template<typename K, typename V>
K Node<K, V> :: get_Key() const{
  return key;
};

template<typename K, typename V>
V Node<K, V>::get_value() const {
  return value;
};

template<typename K, typename V>
void Node<K, V>::set_value(V value){
  this->value = value;
};

template<typename  K, typename  V>
class SkipList{
public:
  SkipList(int);
  ~SkipList();
  int get_random_level();
  Node<K, V>* create_node(K, V, int);
  int insert_element(K, V);
  void display_list();
  bool search_element(K);
  void delete_element(K);
  void dump_file();
  void load_file();
  int size();

private:
  void get_key_value_from_string(const std::string& str, std::string* key, std::string& value);
  bool is_valid_string(const std::string& str);

private:
  int _max_level;
  int _skip_list_level;

  Node<K, V> * _header;

  std::ofstream _file_writer;
  std::ifstream _file_reader;

  int _element_count;
};


template<typename K, typename V>
Node<K, V>* SkipList<K,V>::create_node(const K k , const V v, int level) {
  Node<K, V> *n = new Node<K, V>(k, v, level);
  return n;
};


template<typename  K, typename  V>
int SkipList<K, V>::insert_element(const K key, const V value) {
  mtx.lock();
  Node<K, V>* current = this->_header;

  Node<K, V>* update[_max_level+1];
  memset(update, 0, sizeof(Node<K, V>*)*(_max_level+1));

  for(int i = _skip_list_level; i>= 0; i--){
    while(current->forward[i] != NULL && current->forward[i]->get_key() < key){
      current = current->forward[i];
    }
    update[i] = current;
  }

  //到达了最后一层, 并且找到了比要插入 key 值大的的位置
  current = current->forward[0];

  //插入 key 已经存在
  if(current != NULL && current->get_value() == key){
    std::cout << "key: " << key << "已存在"<<std::endl;
    mtx.unlock();
    return 1;
  }
  //key 不存在就插入
  if(current == NULL || current->get_value() != key){ //注意是"或" 的逻辑
    int random_level = get_random_level();

    //如果随机层>当前层, 向上构建空层数, 并且更新层数信息
    if(random_level > _skip_list_level){
      for(int i = _skip_list_level + 1; i < random_level + 1; i++){
        update[i] = _header;
      }
      _skip_list_level = random_level;
    }
    //创建新节点
    Node<K, V>* insert_node = create_node(key, value, random_level);

    //插入节点
    for(int i = 0; i <= random_level; i++){
      insert_node->forward[i] = update[i]->forward[i];
      update[i]->forward[i] = insert_node;
  }
  std::cout<<"成功插入 key: "<<key<<", value: "<<value<<std::endl;
  _element_count++;
  }
  mtx.unlock();
  return 0;
}

//展示链表
//template<typename  K, typename V>
//void SkipList<K, V>::display_list() {
//  std::cout<< "\n******* Skip List*******" <<"\n";
//  for(int i = 0; i <= _skip_list_level; i++){
//    Node<K,V> *node = this->_header->forward[i];
//    st
//  }
//}






