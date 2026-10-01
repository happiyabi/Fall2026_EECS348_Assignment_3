// CEO Email Prioritizer
// Uses a hand-built array-based MaxHeap as a priority queue.
// Build: g++ -std=c++11 -o ceo_email ceo_email.cpp
// Run:   ./ceo_email testfile.txt   (or run with no args and enter the file name)

#include <iostream>
#include <fstream>
#include <string>

using namespace std;

// ---------------------------------------------------------------------------
// Email: holds one message and knows how to compare its priority to another.
// ---------------------------------------------------------------------------
class Email
{
private:
    string sender;
    string subject;
    string date;    // MM-DD-YYYY
    int senderRank; // higher = read sooner
    int dateKey;    // YYYYMMDD, higher = newer
    long arrival;   // tie-breaker: earlier arrival wins

    static int rankFor(const string &s)
    {
        if (s == "Boss")
            return 5;
        if (s == "Subordinate")
            return 4;
        if (s == "Peer")
            return 3;
        if (s == "ImportantPerson")
            return 2;
        return 1; // OtherPerson
    }

    static int keyFor(const string &d)
    {
        // MM-DD-YYYY -> YYYYMMDD
        if (d.size() < 10)
            return 0;
        int mm = stoi(d.substr(0, 2));
        int dd = stoi(d.substr(3, 2));
        int yyyy = stoi(d.substr(6, 4));
        return yyyy * 10000 + mm * 100 + dd;
    }

public:
    Email() : senderRank(0), dateKey(0), arrival(0) {}

    Email(const string &snd, const string &subj, const string &dt, long order)
        : sender(snd), subject(subj), date(dt),
          senderRank(rankFor(snd)), dateKey(keyFor(dt)), arrival(order) {}

    // true if this email should be read BEFORE other
    bool higherPriorityThan(const Email &other) const
    {
        if (senderRank != other.senderRank)
            return senderRank > other.senderRank;
        if (dateKey != other.dateKey)
            return dateKey > other.dateKey; // newest first
        return arrival < other.arrival;     // stable tie-break
    }

    void display() const
    {
        cout << "Next email:" << endl;
        cout << "\tSender: " << sender << endl;
        cout << "\tSubject: " << subject << endl;
        cout << "\tDate: " << date << endl;
        cout << endl;
    }
};

// ---------------------------------------------------------------------------
// MaxHeap: list-based (dynamic array) binary max-heap of Emails.
// ---------------------------------------------------------------------------
class MaxHeap
{
private:
    Email *data;
    int count;
    int capacity;

    static int parent(int i) { return (i - 1) / 2; }
    static int left(int i) { return 2 * i + 1; }
    static int right(int i) { return 2 * i + 2; }

    void grow()
    {
        int newCap = capacity * 2;
        Email *bigger = new Email[newCap];
        for (int i = 0; i < count; i++)
            bigger[i] = data[i];
        delete[] data;
        data = bigger;
        capacity = newCap;
    }

    void swapAt(int a, int b)
    {
        Email tmp = data[a];
        data[a] = data[b];
        data[b] = tmp;
    }

    void siftUp(int i)
    {
        while (i > 0 && data[i].higherPriorityThan(data[parent(i)]))
        {
            swapAt(i, parent(i));
            i = parent(i);
        }
    }

    void siftDown(int i)
    {
        while (true)
        {
            int l = left(i), r = right(i), best = i;
            if (l < count && data[l].higherPriorityThan(data[best]))
                best = l;
            if (r < count && data[r].higherPriorityThan(data[best]))
                best = r;
            if (best == i)
                break;
            swapAt(i, best);
            i = best;
        }
    }

public:
    MaxHeap() : data(new Email[16]), count(0), capacity(16) {}

    ~MaxHeap() { delete[] data; }

    // Disable copying (owns raw memory)
    MaxHeap(const MaxHeap &) = delete;
    MaxHeap &operator=(const MaxHeap &) = delete;

    void insert(const Email &e)
    {
        if (count == capacity)
            grow();
        data[count] = e;
        siftUp(count);
        count++;
    }

    bool isEmpty() const { return count == 0; }
    int size() const { return count; }

    // Caller must check isEmpty() first.
    const Email &peekMax() const { return data[0]; }

    void removeMax()
    {
        if (count == 0)
            return;
        data[0] = data[count - 1];
        count--;
        if (count > 0)
            siftDown(0);
    }
};

// ---------------------------------------------------------------------------
// Inbox: the CEO's inbox; wraps the heap and executes file commands.
// ---------------------------------------------------------------------------
class Inbox
{
private:
    MaxHeap heap;
    long nextOrder;

    static string trim(const string &s)
    {
        size_t a = s.find_first_not_of(" \t\r\n");
        if (a == string::npos)
            return "";
        size_t b = s.find_last_not_of(" \t\r\n");
        return s.substr(a, b - a + 1);
    }

    void addEmail(const string &fields)
    {
        // fields: "<sender>,<subject>,<date>"
        size_t c1 = fields.find(',');
        size_t c2 = (c1 == string::npos) ? string::npos : fields.find(',', c1 + 1);
        if (c1 == string::npos || c2 == string::npos)
            return; // malformed line
        string sender = trim(fields.substr(0, c1));
        string subject = trim(fields.substr(c1 + 1, c2 - c1 - 1));
        string date = trim(fields.substr(c2 + 1));
        heap.insert(Email(sender, subject, date, nextOrder++));
    }

public:
    Inbox() : nextOrder(0) {}

    void count() const
    {
        cout << "There are " << heap.size() << " emails to read." << endl;
        cout << endl;
    }

    void next() const
    {
        if (heap.isEmpty())
        {
            cout << "No emails to read." << endl;
            cout << endl;
            return;
        }
        heap.peekMax().display();
    }

    void read()
    {
        heap.removeMax(); // no-op if empty
    }

    void processLine(const string &rawLine)
    {
        string line = trim(rawLine);
        if (line.empty())
            return;

        if (line.compare(0, 6, "EMAIL ") == 0)
        {
            addEmail(line.substr(6));
        }
        else if (line == "NEXT")
        {
            next();
        }
        else if (line == "READ")
        {
            read();
        }
        else if (line == "COUNT")
        {
            count();
        }
        // unknown lines are ignored
    }
};

// ---------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    string filename;
    if (argc > 1)
    {
        filename = argv[1];
    }
    else
    {
        cout << "Enter test file name: ";
        getline(cin, filename);
    }

    ifstream in(filename);
    if (!in)
    {
        cerr << "Could not open file: " << filename << endl;
        return 1;
    }

    Inbox inbox;
    string line;
    while (getline(in, line))
    {
        inbox.processLine(line);
    }
    return 0;
}