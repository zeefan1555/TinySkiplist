
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <mutex>
#include <fstream>
#include <string>
#define STORE_FILE "strore/dumpFile"
using  namespace  std;

std::mutex mtx;     //锁
std::string delimiter = ":";


template<typename K, typename V>
class Node{
public:
  Node(){};
  Node(K k, V v, int);
  ~Node();
  K get_key() const;
  V get_value() const;

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
K Node<K, V> :: get_key() const{
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
  void get_key_value_from_string(const std::string& str, std::string* key, std::string* value);
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
    while(current->forward[i] != NULL && current->forward[i]->get_key() < key){ //在每一层中, 达到最后一个元素 "或者"  >= key 时停止
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
template<typename  K, typename V>
void SkipList<K, V>::display_list() {
  std::cout<< "\n******* Skip List*******" <<"\n";
  for(int i = _skip_list_level; i >= 0; i-- ){
    Node<K,V> *node = this->_header->forward[i];
    cout<<"level "<< i << ": ";
    while(node != NULL){
      cout << node->get_key()<< ":" << node->get_value() << ";";
      node = node->forward[i];
    }
    cout << endl;

  }
}

//从内存中读取数据

template<typename  K, typename V>
void SkipList<K, V>::dump_file(){
  cout << "--------------dump_file------------------" << endl;
  _file_writer.open(STORE_FILE);
  Node<K,V> *node = this->_header->forward[0];

  while(node != NULL){
    _file_writer<< node->get_key() << ":" << node->get_value()<<"\n";
    cout<< node->get_key() << ":" << node->get_value() << "; \n";
    node = node->forward[0];
  }
  _file_writer.flush();//❓
  _file_writer.close();
  return ;
}

//缓存到磁盘
template<typename  K, typename  V>
void SkipList<K, V> ::load_file() {
  _file_reader.open(STORE_FILE);
  cout<<"-------------load_file-------------" <<endl;
  std::string line;
  std::string* key = new string();
  std::string* value = new string();
  while(getline(_file_reader, line)){
    get_key_value_from_string(line, key, value);
    if(key->empty() || value->empty()){
      continue ;
    }
    insert_element(*key, *value);
    cout<<"key:" << *key << "value: " << *value << endl;

  }
  _file_writer.close();
}

//获取当前跳表的数量
template<typename K, typename  V>
int SkipList<K,V> :: size(){
  return _element_count;
}

template<typename K, typename V>
void SkipList<K,V>::get_key_value_from_string(const std::string &str, std::string* key, std::string* value) {
  if(!is_valid_string(str)){
    return;
  }
  *key = str.substr(0, str.find(delimiter));
  *value = str.substr(str.find(delimiter)+1, str.length());
}

template<typename K, typename V>
bool SkipList<K, V> ::is_valid_string(const std::string &str) {
  if(str.empty()){
    return false;
  }
  if(str.find(delimiter) == std::string::npos){ //❓
    return false;
  }
  return true;
}

//删除元素
template<typename  K, typename  V>
void SkipList<K, V> ::delete_element(K key) {
  mtx.lock();
  Node<K, V> *current = this->_header;
  Node<K, V> *update[_max_level+1];
  meset(update, 0, sizeof(Node<K, V>*)*(_max_level+1));

  //从最高层开始
  for(int i = _skip_list_level; i >= 0; i--){
    while(current->forward[i] != NULL && current->forward[i]->get_key() < key){
      current = current->forward[i];
    }
    update[i] = current;
  }

  current = current->forward[0];
  if(current != NULL && current->get_key() == key){
    for(int i = 0; i <= _skip_list_level; i++){
      if(update[i]->forward[i] != current) break;
      update[i]->forward[i] = current->forward[i];
    }
    //移除没有元素的层
    while(_skip_list_level > 0 && _header->forward[_skip_list_level] == 0){
      _skip_list_level--;
    }

    std::cout<< "成功删除了 key" <<key << std::endl;
    _element_count--;
  }
  mtx.unlock();
  return;
}

//搜索元素
template<typename K, typename V>
bool SkipList<K, V> ::search_element(K key) {
  std::cout<< "----------search_elemet------------" << std::endl;
  Node<K, V> *current = _header;

  //从最高层开始
  for(int i = _skip_list_level; i >= 0; i--){
    while(current->forward[i] && current->forward[i]->get_key() < key){
      current = current->forward[i];
    }
  }
  current = current->forward[0];

  if(current && current->get_key() == key){
    std::cout<<"Found Key: " << key << ", Calue :"  << current->get_value()<<std::endl;
    return true;
  }
  std::cout<<"Not Found Key: "<< key << std::endl;
    return false;
}

//构造跳表
template<typename K, typename V>
SkipList<K, V>::SkipList(int max_level) {
  this->_max_level = max_level;
  this->_skip_list_level = 0;
  this->_element_count = 0;

  //创建头结点
  K k;
  V v;
  this->_header = new Node<K,V>(k, v , _max_level);



};

template<typename K, typename V>
SkipList<K, V> ::~SkipList() {
  if(_file_writer.is_open()){
    _file_writer.close();
  }
  if(_file_reader.is_open()){
    _file_reader.close();
  }
  delete _header;
}

template<typename K, typename V>
int SkipList<K,V>::get_random_level() {
  int k = 1;
  while( rand() % 2){ //奇数+1, 0.5 的概率
    k++;
  }
  k = (k < _max_level) ? k : _max_level;
  return k;
};








