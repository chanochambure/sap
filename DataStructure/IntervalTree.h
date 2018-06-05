#ifndef INCLUDED_INTERVAL_TREE_H
#define INCLUDED_INTERVAL_TREE_H

#include "../Object.h"

#include <stack>

struct sweepLeftToRight {
  bool operator() (const Interval& a, const Interval& b) const
  {
    if(a.first == b.first)
        return a.second < b.second;
    return a.first < b.first;
  }
};

struct sweepRightToLeft {
  bool operator() (const Interval& a, const Interval& b) const
  {
    if(a.second == b.second)
        return a.first > b.first;
    return a.second > b.second;
  }
};

class IntervalTree
{
    private:
        struct Node
        {
            std::set<Interval,sweepLeftToRight> increasing;
            std::set<Interval,sweepRightToLeft> decreasing;
            Node* left=nullptr;
            Node* right=nullptr;
            float midpoint=0;
            int tree_height;
            Node(Interval interval)
            {
                decreasing.insert(interval);
                increasing.insert(interval);
                midpoint = interval.getMidpoint();
                tree_height = 1;
            }
            int height()
            {
                return tree_height;
            }
            int height(Node* node){
                return node && node->height();
            }
            Node* balanceOut()
            {
                int balance = height(left) - height(right);
                if (balance < -1){
                    // The tree is right-heavy.
                    if (height(right->left) > height(right->right)){
                        this->right = this->right->rightRotate();
                        return leftRotate();
                    } else{
                        return leftRotate();
                    }
                } else if (balance > 1){
                    // The tree is left-heavy.
                    if (height(left->right) > height(left->left)){
                        this->left = this->left->leftRotate();
                        return rightRotate();
                    } else
                        return rightRotate();
                } else {
                    // The tree is already balanced.
                    return this;
                }
            }
            Node* leftRotate()
            {
                Node* head = right;
                right = head->left;
                head->left = this;
                tree_height = std::max(height(right), height(left)) + 1;
                head->left = head->assimilateOverlappingIntervals(this);
                return head;
            }
            Node* rightRotate()
            {
                Node* head = left;
                left = head->right;
                head->right = this;
                tree_height = std::max(height(right), height(left)) + 1;
                head->right = head->assimilateOverlappingIntervals(this);
                return head;
            }
            Node* assimilateOverlappingIntervals(Node* from)
            {
                std::list<Interval> tmp;
                if (compareTo(midpoint,from->midpoint) < 0){
                    auto i=from->increasing.begin();
                    while(i!=from->increasing.end())
                    {
                        Interval next=*i;
                        if (next.isRightOf(midpoint))
                            break;
                        tmp.push_back(next);
                        ++i;
                    }
                }
                else
                {
                    auto i=from->decreasing.begin();
                    while(i!=from->decreasing.end())
                    {
                        Interval next=*i;
                        if (next.isLeftOf(midpoint))
                            break;
                        tmp.push_back(next);
                        ++i;
                    }
                }
                for(auto i=tmp.begin();i!=tmp.end();++i)
                {
                    Interval next=*i;
                    auto inc=from->increasing.find(next);
                    if(inc != from->increasing.end())
                        from->increasing.erase(inc);
                    auto dec=from->decreasing.find(next);
                    if(dec != from->decreasing.end())
                        from->decreasing.erase(dec);
                }
                increasing.insert(tmp.begin(),tmp.end());
                decreasing.insert(tmp.begin(),tmp.end());
                if (from->increasing.size() == 0){
                    return deleteNode(from);
                }
                return from;
            }
            static Node* deleteNode(Node* root)
            {
                if (root->left == nullptr && root->right == nullptr)
                    return nullptr;

                if (root->left == nullptr)
                    return root->right;
                else
                {
                    Node* node = root->left;
                    std::stack<Node*> new_stack;
                    while (node->right != nullptr){
                        new_stack.push(node);
                        node = node->right;
                    }
                    if (!new_stack.empty()) {
                        new_stack.top()->right = node->left;
                        node->left = root->left;
                    }
                    node->right = root->right;

                    Node* newRoot = node;
                    while (!new_stack.empty()){
                        node = new_stack.top();
                        new_stack.pop();
                        if (!new_stack.empty())
                            new_stack.top()->right = newRoot->assimilateOverlappingIntervals(node);
                        else
                            newRoot->left = newRoot->assimilateOverlappingIntervals(node);
                    }
                    return newRoot->balanceOut();
                }
            }
            static Node* addInterval(IntervalTree* tree, Node* root, Interval interval)
            {
                if (root == nullptr) {
                    tree->size++;
                    return new Node(interval);
                }
                if (interval.contains(root->midpoint))
                {
                    auto v=root->decreasing.insert(interval);
                    if(v.second)
                        tree->size++;
                    root->increasing.insert(interval);
                    return root;
                } else if (interval.isLeftOf(root->midpoint)){
                    root->left = addInterval(tree, root->left, interval);
                    root->tree_height = std::max(root->height(root->left), root->height(root->right))+1;
                } else {
                    root->right = addInterval(tree, root->right, interval);
                    root->tree_height = std::max(root->height(root->left), root->height(root->right))+1;
                }

                return root->balanceOut();
            }
            static Node* removeInterval(IntervalTree* tree, Node* root, Interval interval)
            {
                if (root == nullptr)
                    return nullptr;
                if (interval.contains(root->midpoint)){
                    auto iter=root->decreasing.find(interval);
                    if(iter!=root->decreasing.end())
                    {
                        root->decreasing.erase(iter);
                        tree->size--;
                    }
                    auto iter2=root->increasing.find(interval);
                    if(iter2!=root->increasing.end())
                        root->increasing.erase(iter2);
                    if (root->increasing.size() == 0){
                        return deleteNode(root);
                    }

                } else if (interval.isLeftOf(root->midpoint)){
                    root->left = removeInterval(tree, root->left, interval);
                } else {
                    root->right = removeInterval(tree, root->right, interval);
                }
                return root->balanceOut();
            }
            static void rangeQueryLeft(Node* node, Interval query, std::set<Interval>& result)
            {
                while (node != nullptr)
                {
                    if (query.contains(node->midpoint)) {
                        result.insert(node->increasing.begin(),node->increasing.end());
                        if (node->right)
                        {
                            Node::iterator iter=node->right->begin();
                            while(iter.hasNext())
                            {
                                result.insert(iter.get());
                                iter.next();
                            }
                        }
                        node = node->left;
                    } else {
                        for(auto i=node->decreasing.begin();i!=node->decreasing.end();++i)
                        {
                            Interval next=*i;
                            if (next.isLeftOf(query))
                                break;
                            result.insert(next);
                        }
                        node = node->right;
                    }
                }
            }
            static void rangeQueryRight(Node* node, Interval query, std::set<Interval>& result)
            {
                while (node)
                {
                    if (query.contains(node->midpoint)) {
                        result.insert(node->increasing.begin(),node->increasing.end());
                        if (node->left)
                        {
                            Node::iterator iter=node->left->begin();
                            while(iter.hasNext())
                            {
                                result.insert(iter.get());
                                iter.next();
                            }
                        }
                        node = node->right;
                    } else {
                        for(auto i=node->increasing.begin();i!=node->increasing.end();++i)
                        {
                            Interval next=*i;
                            if (next.isRightOf(query))
                                break;
                            result.insert(next);
                        }
                        node = node->left;
                    }
                }
            }
            struct iterator
            {
                std::stack<Node*> main_stack;
                Node* root=nullptr;
                Node* current_node=nullptr;
                std::set<Interval>::iterator main_iterator;
                std::set<Interval>::iterator end_iterator;
                iterator(Node* node)
                {
                    if(node)
                    {
                        for(root=node;root;root=root->left)
                            main_stack.push(root);
                        current_node = main_stack.top();
                        main_stack.pop();
                        main_iterator = current_node->increasing.begin();
                        end_iterator = current_node->increasing.end();
                        root = current_node->right;
                    }
                }
                bool hasNext()
                {
                    return root || !main_stack.empty() || main_iterator!=end_iterator;
                }
                void next()
                {
                    ++main_iterator;
                    if(main_iterator==end_iterator)
                    {
                        for(;root;root=root->left)
                            main_stack.push(root);
                        if(! main_stack.empty())
                        {
                            current_node = main_stack.top();
                            main_stack.pop();
                            main_iterator = current_node->increasing.begin();
                            end_iterator = current_node->increasing.end();
                            root = current_node->right;
                        }
                    }
                }
                Interval get()
                {
                    return *main_iterator;
                }
            };
            iterator begin()
            {
                return iterator(this);
            }
        };
        unsigned int size=0;
        Node* root=nullptr;
        public:
        IntervalTree()
        {
        }
        bool insert(Interval interval){
            unsigned int sizeBeforeOperation = size;
            root = Node::addInterval(this, root, interval);
            return size != sizeBeforeOperation;
        }
        std::set<Interval> query(Interval interval){
            std::set<Interval> result;
            Node* node = root;
            while(node != nullptr)
            {
                if (interval.contains(node->midpoint))
                {
                    result.insert(node->increasing.begin(),node->increasing.end());
                    Node::rangeQueryLeft(node->left, interval, result);
                    Node::rangeQueryRight(node->right, interval, result);
                    break;
                }
                if (interval.isLeftOf(node->midpoint))
                {
                    for(auto i=node->increasing.begin();i!=node->increasing.end();++i)
                    {
                        Interval next=*i;
                        if (!interval.intersects(next))
                            break;
                        result.insert(next);
                    }
                    node = node->left;
                }
                else
                {
                    for(auto i=node->decreasing.begin();i!=node->decreasing.end();++i)
                    {
                        Interval next=*i;
                        if (!interval.intersects(next))
                            break;
                        result.insert(next);
                    }
                    node = node->right;
                }
            }
            return result;
        }
        bool remove(Interval interval)
        {
            if (root == nullptr)
                return false;
            unsigned int sizeBeforeOperation = size;
            root = Node::removeInterval(this, root, interval);
            return size != sizeBeforeOperation;
        }
        unsigned int get_size()
        {
            return size;
        }
};

#endif // INCLUDED_INTERVAL_TREE_H
