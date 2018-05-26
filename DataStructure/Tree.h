#ifndef INCLUDED_TREE_H
#define INCLUDED_TREE_H

#include <stack>
#include <iostream>

template<typename T>
class Tree
{
    private:
        struct _S_Node
        {
            T data;
            _S_Node* next=nullptr;
            _S_Node* back=nullptr;
            ~_S_Node()
            {
                if(back)
                    delete(back);
                if(next)
                    delete(next);
            }
        };
        _S_Node* _V_root=nullptr;
        bool _F_find(T data,_S_Node**& ini)
        {
            bool return_value=false;
            while(*ini)
            {
                if((*ini)->data==data)
                {
                    return_value=true;
                    break;
                }
                if((*ini)->data<data)
                    ini=&((*ini)->next);
                else
                    ini=&((*ini)->back);
            }
            return return_value;
        }
        T min_data;
    public:
        class iterator
        {
            private:
                std::stack<std::pair<_S_Node*,int>> _V_stack;
                void _F_get_next()
                {
                    while(!_V_stack.empty())
                    {
                        int& status=(_V_stack.top().second);
                        if(status==0)
                        {
                            ++status;
                            if(_V_stack.top().first->back)
                                _V_stack.push(std::pair<_S_Node*,int>(_V_stack.top().first->back,0));
                        }
                        else if(status==1)
                        {
                            ++status;
                            break;
                        }
                        if(status==2)
                        {
                            ++status;
                            if(_V_stack.top().first->next)
                                _V_stack.push(std::pair<_S_Node*,int>(_V_stack.top().first->next,0));
                        }
                        else if(status==3)
                        {
                            _V_stack.pop();
                        }
                    }
                }
            public:
                iterator(){}
                iterator(_S_Node* root_node)
                {
                    if(root_node)
                        _V_stack.push(std::pair<_S_Node*,int>(root_node,0));
                    _F_get_next();
                }
                iterator(_S_Node* root_node,T data)
                {
                    while(root_node)
                    {
                        if(root_node->data==data)
                        {
                            _V_stack.push(std::pair<_S_Node*,int>(root_node,2));
                            break;
                        }
                        if(root_node->data>data)
                        {
                            _V_stack.push(std::pair<_S_Node*,int>(root_node,1));
                            root_node=root_node->back;
                        }
                        else if(root_node->data<data)
                        {
                            _V_stack.push(std::pair<_S_Node*,int>(root_node,3));
                            root_node=root_node->next;
                        }
                    }
                    if(!_V_stack.empty())
                        _V_stack.top().second=2;
                }
                iterator operator ++ (int)
                {
                    iterator temp_iterator=(*this);
                    _F_get_next();
                    return temp_iterator;
                }
                iterator operator ++ ()
                {
                    _F_get_next();
                    return (*this);
                }
                const T operator * ()
                {
                    return _V_stack.top().first->data;
                }
                bool operator != (iterator another_iterator)
                {
                    return (_V_stack!=another_iterator._V_stack);
                }
        };
        bool find(T data)
        {
            _S_Node** ini=&_V_root;
            return _F_find(data,ini);
        }
        bool insert(T data)
        {
            if(!_V_root)
                min_data=data;
            _S_Node** ini=&_V_root;
            if(_F_find(data,ini))
                return false;
            _S_Node* new_node=new _S_Node;
            new_node->data=data;
            *ini=new_node;
            if(min_data>data)
                min_data=data;
            return true;
        }
        bool remove(T data)
        {
            _S_Node** ini=&_V_root;
            if(!_F_find(data,ini))
                return false;
            if(((*ini)->next) && ((*ini)->back))
            {
                _S_Node** last_node=&((*ini)->next);
                while((*last_node)->back)
                    last_node=&((*last_node)->back);
                (*ini)->data=(*last_node)->data;
                ini=last_node;
                if((*ini)->next)
                {
                    _S_Node* temp=*ini;
                    *ini=(*ini)->next;
                    temp->next=nullptr;
                    delete(temp);
                }
                else
                {
                    delete(*ini);
                    *ini=nullptr;
                }
            }
            else if((*ini)->next)
            {
                _S_Node* temp=*ini;
                *ini=(*ini)->next;
                temp->next=nullptr;
                delete(temp);
            }
            else if((*ini)->back)
            {
                _S_Node* temp=*ini;
                *ini=(*ini)->back;
                temp->back=nullptr;
                delete(temp);
            }
            else
            {
                delete(*ini);
                *ini=nullptr;
            }
            return true;
        }
        iterator begin()
        {
            return iterator(_V_root);
        }
        iterator end()
        {
            return iterator();
        }
        std::pair<iterator,iterator> range_query(T ini_data,T fin_data,bool open=false)
        {
            if(ini_data>fin_data)
            {
                T temp=fin_data;
                fin_data=ini_data;
                ini_data=temp;
            }
            auto return_value=std::pair<iterator,iterator>(iterator(_V_root,ini_data),iterator(_V_root,fin_data));
            if(open)
            {
                if(return_value.first!=end() && *(return_value.first)==ini_data)
                    ++(return_value.first);
            }
            else
            {
                if(return_value.second!=end() && *(return_value.second)==fin_data)
                    ++(return_value.second);
            }
            return return_value;
        }
        ~Tree()
        {
            if(_V_root)
                delete(_V_root);
        }
};

#endif // INCLUDED_TREE_H
