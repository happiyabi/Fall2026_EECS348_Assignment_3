#include <iostream>
#include <string>
#include <sstream>
#include <fstream>

// Class representing an Email object
class Email
{
private:
    std::string senderCategory;
    std::string subject;
    std::string date; // MM-DD-YYYY

    // Helper to assign numeric weight to categories
    int getCategoryPriority() const
    {
        if (senderCategory == "Boss")
            return 5;
        if (senderCategory == "Subordinate")
            return 4;
        if (senderCategory == "Peer")
            return 3;
        if (senderCategory == "ImportantPerson")
            return 2;
        if (senderCategory == "OtherPerson")
            return 1;
        return 0;
    }

    // Helper to convert MM-DD-YYYY into YYYYMMDD string for simple chronological comparison
    std::string getFormattedDate() const
    {
        if (date.length() == 10)
        {
            std::string month = date.substr(0, 2);
            std::string day = date.substr(3, 2);
            std::string year = date.substr(6, 4);
            return year + month + day;
        }
        return date;
    }

public:
    Email() : senderCategory(""), subject(""), date("") {}

    Email(const std::string &sender, const std::string &subj, const std::string &d)
        : senderCategory(sender), subject(subj), date(d) {}

    std::string getSenderCategory() const { return senderCategory; }
    std::string getSubject() const { return subject; }
    std::string getDate() const { return date; }

    // Custom comparison operators based on priority hierarchy & newest date
    bool operator>(const Email &other) const
    {
        if (getCategoryPriority() != other.getCategoryPriority())
        {
            return getCategoryPriority() > other.getCategoryPriority();
        }
        return getFormattedDate() > other.getFormattedDate();
    }

    bool operator<(const Email &other) const
    {
        if (getCategoryPriority() != other.getCategoryPriority())
        {
            return getCategoryPriority() < other.getCategoryPriority();
        }
        return getFormattedDate() < other.getFormattedDate();
    }
};

// Node structure for pointer/list-based tree nodes
struct Node
{
    Email data;
    Node *left;
    Node *right;
    Node *parent;

    Node(const Email &email)
        : data(email), left(nullptr), right(nullptr), parent(nullptr) {}
};

// Dynamic linked queue used strictly for Level-Order tree traversal
class NodeQueue
{
private:
    struct QueueNode
    {
        Node *treeNode;
        QueueNode *next;
        QueueNode(Node *node) : treeNode(node), next(nullptr) {}
    };

    QueueNode *head;
    QueueNode *tail;

public:
    NodeQueue() : head(nullptr), tail(nullptr) {}

    ~NodeQueue()
    {
        while (!isEmpty())
        {
            dequeue();
        }
    }

    bool isEmpty() const
    {
        return head == nullptr;
    }

    void enqueue(Node *node)
    {
        QueueNode *newNode = new QueueNode(node);
        if (tail == nullptr)
        {
            head = tail = newNode;
        }
        else
        {
            tail->next = newNode;
            tail = newNode;
        }
    }

    Node *dequeue()
    {
        if (isEmpty())
            return nullptr;
        QueueNode *temp = head;
        Node *node = temp->treeNode;
        head = head->next;
        if (head == nullptr)
        {
            tail = nullptr;
        }
        delete temp;
        return node;
    }
};

// Pointer-based (list-based) MaxHeap implementation
class MaxHeap
{
private:
    Node *root;
    int count;

    void swapData(Node *a, Node *b)
    {
        Email temp = a->data;
        a->data = b->data;
        b->data = temp;
    }

    void heapifyUp(Node *node)
    {
        while (node->parent != nullptr && node->data > node->parent->data)
        {
            swapData(node, node->parent);
            node = node->parent;
        }
    }

    void heapifyDown(Node *node)
    {
        while (node != nullptr)
        {
            Node *largest = node;

            if (node->left != nullptr && node->left->data > largest->data)
            {
                largest = node->left;
            }
            if (node->right != nullptr && node->right->data > largest->data)
            {
                largest = node->right;
            }

            if (largest != node)
            {
                swapData(node, largest);
                node = largest;
            }
            else
            {
                break;
            }
        }
    }

    // Level-order helper to locate insertion parent node
    Node *getInsertionParent()
    {
        if (root == nullptr)
            return nullptr;

        NodeQueue q;
        q.enqueue(root);

        while (!q.isEmpty())
        {
            Node *curr = q.dequeue();
            if (curr->left == nullptr || curr->right == nullptr)
            {
                return curr;
            }
            q.enqueue(curr->left);
            q.enqueue(curr->right);
        }
        return nullptr;
    }

    // Level-order helper to find the last node in complete binary tree
    Node *getLastNode()
    {
        if (root == nullptr)
            return nullptr;

        NodeQueue q;
        q.enqueue(root);
        Node *last = nullptr;

        while (!q.isEmpty())
        {
            last = q.dequeue();
            if (last->left)
                q.enqueue(last->left);
            if (last->right)
                q.enqueue(last->right);
        }
        return last;
    }

    void clearTree(Node *node)
    {
        if (node != nullptr)
        {
            clearTree(node->left);
            clearTree(node->right);
            delete node;
        }
    }

public:
    MaxHeap() : root(nullptr), count(0) {}

    ~MaxHeap()
    {
        clearTree(root);
    }

    int size() const
    {
        return count;
    }

    bool isEmpty() const
    {
        return count == 0;
    }

    void insert(const Email &email)
    {
        Node *newNode = new Node(email);
        count++;

        if (root == nullptr)
        {
            root = newNode;
            return;
        }

        Node *parent = getInsertionParent();
        newNode->parent = parent;

        if (parent->left == nullptr)
        {
            parent->left = newNode;
        }
        else
        {
            parent->right = newNode;
        }

        heapifyUp(newNode);
    }

    Email getMax() const
    {
        if (isEmpty())
        {
            return Email();
        }
        return root->data;
    }

    void removeMax()
    {
        if (isEmpty())
            return;

        if (count == 1)
        {
            delete root;
            root = nullptr;
            count = 0;
            return;
        }

        Node *lastNode = getLastNode();
        root->data = lastNode->data;

        if (lastNode->parent != nullptr)
        {
            if (lastNode->parent->left == lastNode)
            {
                lastNode->parent->left = nullptr;
            }
            else
            {
                lastNode->parent->right = nullptr;
            }
        }

        delete lastNode;
        count--;

        heapifyDown(root);
    }
};

// CEO Inbox Controller Object managing command parsing
class CEOInboxManager
{
private:
    MaxHeap priorityQueue;

public:
    void processCommand(const std::string &line)
    {
        if (line.empty())
            return;

        if (line.rfind("EMAIL ", 0) == 0)
        {
            std::string payload = line.substr(6);
            std::stringstream ss(payload);
            std::string sender, subject, date;

            std::getline(ss, sender, ',');
            std::getline(ss, subject, ',');
            std::getline(ss, date, ',');

            Email newEmail(sender, subject, date);
            priorityQueue.insert(newEmail);
        }
        else if (line == "COUNT")
        {
            std::cout << "There are " << priorityQueue.size() << " emails to read." << std::endl
                      << std::endl;
        }
        else if (line == "NEXT")
        {
            if (!priorityQueue.isEmpty())
            {
                Email top = priorityQueue.getMax();
                std::cout << "Next email:" << std::endl;
                std::cout << "    Sender: " << top.getSenderCategory() << std::endl;
                std::cout << "    Subject: " << top.getSubject() << std::endl;
                std::cout << "    Date: " << top.getDate() << std::endl
                          << std::endl;
            }
        }
        else if (line == "READ")
        {
            if (!priorityQueue.isEmpty())
            {
                priorityQueue.removeMax();
            }
        }
    }
};

int main(int argc, char *argv[])
{
    CEOInboxManager inbox;

    // Read commands from file if passed as command line argument, else fallback to standard input
    if (argc > 1)
    {
        std::ifstream inputFile(argv[1]);
        if (inputFile.is_open())
        {
            std::string line;
            while (std::getline(inputFile, line))
            {
                inbox.processCommand(line);
            }
            inputFile.close();
            return 0;
        }
    }

    std::string line;
    while (std::getline(std::cin, line))
    {
        inbox.processCommand(line);
    }

    return 0;
}