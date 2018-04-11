#ifndef INCLUDED_LIST_H
#define INCLUDED_LIST_H

template<typename T>
class List
{
    private:
        struct _S_Node
        {
            T data;
            _S_Node* next=nullptr;
            ~_S_Node()
            {
                if(next)
                    delete(next);
            }
        };
        _S_Node* _V_root=nullptr;
        bool _F_find(T data,_S_Node**& ini)
        {
            for(ini=&_V_root;(*ini) and ((*ini)->data)<data;ini=&((*ini)->next));
            return (*ini) and (*ini)->data==data;
        }
    public:
        class iterator
        {
            private:
                _S_Node* _V_root_node=nullptr;
                void _F_get_next()
                {
                    _V_root_node=_V_root_node->next;
                }
            public:
                iterator(_S_Node* root_node=nullptr)
                {
                    _V_root_node=root_node;
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
                    return _V_root_node->data;
                }
                bool operator != (iterator another_iterator)
                {
                    return (_V_root_node!=another_iterator._V_root_node);
                }
        };
        bool find(T data)
        {
            _S_Node** ini;
            return _F_find(data,ini);
        }
        bool insert(T data)
        {
            _S_Node** ini;
            if(_F_find(data,ini))
                return false;
            _S_Node* new_node=new _S_Node;
            new_node->data=data;
            new_node->next=*ini;
            *ini=new_node;
            return true;
        }
        bool remove(T data)
        {
            _S_Node** ini;
            if(!_F_find(data,ini))
                return false;
            _S_Node* temp_node=*ini;
            *ini=temp_node->next;
            temp_node->next=nullptr;
            delete(temp_node);
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
            _S_Node** ini;
            _F_find(ini_data,ini);
            if(open and (*ini) and (*ini)->data==ini_data)
                ini=&((*ini)->next);
            _S_Node** fin;
            _F_find(fin_data,fin);
            if(!open and (*fin) and (*fin)->data==fin_data)
                fin=&((*fin)->next);
            return std::pair<iterator,iterator>(iterator(*ini),iterator(*fin));
        }
};

#endif // INCLUDED_LIST_H
