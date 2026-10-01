/*
Program: EECS 348 Assignment 3
Description: Prioritizes a CEO's emails using a from scratch, array-based
             MaxHeap. Orders by sender category (Boss > Subordinate > Peer
             > ImportantPerson > OtherPerson), then by newest date
Inputs: Commands from a file given on the command line or from standard
        input (works with < redirection): EMAIL <category>,<subject>,<date>,
        NEXT, READ, COUNT
Outputs: Terminal output for NEXT/COUNT; "No emails to read." when NEXT or
         READ is used on an empty inbox; stderr warnings for bad input
Collaborators: None
Other Sources: None
Author: Yabsira Tamirat
Creation Date: October 1, 2026
Revised: October 1, 2026
Revisions: Added input validation (category, date, missing fields) with
           stderr warnings; reads from a file argument or standard input
           instead of prompting; NEXT and READ now print a message when the
           inbox is empty; heap array held by unique_ptr for safer memory;
           renamed classes from the original draft; strips Windows
           line endings and extra spaces around fields
*/

#include <iostream> // for cout, cerr, cin
#include <fstream>  // for ifstream (reading a file)
#include <string>   // for string and getline
#include <memory>   // for unique_ptr (automatic memory cleanup)

using namespace std; // lets me write string instead of std::string

// Starting size of the heap array (it doubles when it fills up)
const int INITIAL_CAPACITY = 16;

// ---------------------------------------------------------------------------
// Message: one email in the CEO's inbox
// ---------------------------------------------------------------------------
class Message
{
private:
    string sender;      // sender category such as Boss or Peer
    string subject;     // subject line of the email
    string date;        // date text in MM-DD-YYYY format
    int categoryRank;   // number for the category (5 = Boss ... 1 = OtherPerson)
    int dateValue;      // date turned into YYYYMMDD so bigger means newer
    long arrivalNumber; // order the email arrived, used to break ties

public:
    // Default constructor: needed so I can make an array of Message objects
    Message()
    {
        sender = "";       // start with an empty sender
        subject = "";      // start with an empty subject
        date = "";         // start with an empty date
        categoryRank = 0;  // rank 0 means "no rank yet"
        dateValue = 0;     // date value 0 means "no date yet"
        arrivalNumber = 0; // arrival number 0 by default
    }

    // Constructor that fills in every field (rank and date value are already worked out)
    Message(string s, string subj, string d, int rank, int value, long arrival)
    {
        sender = s;              // save the sender category
        subject = subj;          // save the subject line
        date = d;                // save the date text
        categoryRank = rank;     // save the category rank
        dateValue = value;       // save the YYYYMMDD number
        arrivalNumber = arrival; // save the arrival order
    }

    // Getter for the sender category
    string getSender() const { return sender; }

    // Getter for the subject line
    string getSubject() const { return subject; }

    // Getter for the date text
    string getDate() const { return date; }

    // Returns true if this email should be read BEFORE the other email
    bool comesBefore(const Message &other) const
    {
        if (categoryRank != other.categoryRank)
        {                                             // different categories?
            return categoryRank > other.categoryRank; // higher category goes first
        }
        if (dateValue != other.dateValue)
        {                                       // same category, different dates?
            return dateValue > other.dateValue; // newer date goes first
        }
        return arrivalNumber < other.arrivalNumber; // same category and date: earlier arrival first
    }

    // Prints this email in the format the assignment asks for
    void print() const
    {
        cout << "Next email:" << endl;            // heading line
        cout << "\tSender: " << sender << endl;   // sender line
        cout << "\tSubject: " << subject << endl; // subject line
        cout << "\tDate: " << date << endl;       // date line
        cout << endl;                             // blank line after the email
    }
};

// ---------------------------------------------------------------------------
// EmailHeap: MaxHeap built from scratch on a dynamic array
// ---------------------------------------------------------------------------
class EmailHeap
{
private:
    unique_ptr<Message[]> items; // the array that holds the heap (freed automatically)
    int count;                   // how many emails are in the heap right now
    int capacity;                // how many emails the array can hold

    // Index of the parent of position i
    int parentOf(int i) const { return (i - 1) / 2; }

    // Index of the left child of position i
    int leftOf(int i) const { return 2 * i + 1; }

    // Index of the right child of position i
    int rightOf(int i) const { return 2 * i + 2; }

    // Makes the array twice as big when it is full
    void doubleCapacity()
    {
        int newCapacity = capacity * 2;                         // new size is double
        unique_ptr<Message[]> bigger(new Message[newCapacity]); // make the bigger array
        for (int i = 0; i < count; i++)
        {                         // go through every email
            bigger[i] = items[i]; // copy it into the bigger array
        }
        items = move(bigger);   // swap in the bigger array (old one is freed)
        capacity = newCapacity; // remember the new size
    }

    // Swaps the emails at positions a and b
    void swapItems(int a, int b)
    {
        Message temp = items[a]; // hold email a
        items[a] = items[b];     // put b's email in a's spot
        items[b] = temp;         // put the held email in b's spot
    }

    // Moves the email at position i up until the heap order is right
    void moveUp(int i)
    {
        while (i > 0 && items[i].comesBefore(items[parentOf(i)]))
        {                              // still beats its parent?
            swapItems(i, parentOf(i)); // swap with the parent
            i = parentOf(i);           // continue from the parent's spot
        }
    }

    // Moves the email at position i down until the heap order is right
    void moveDown(int i)
    {
        while (true)
        {                           // repeat until it stops moving
            int left = leftOf(i);   // left child position
            int right = rightOf(i); // right child position
            int best = i;           // assume this email is the best
            if (left < count && items[left].comesBefore(items[best]))
            {                // left child better?
                best = left; // left child is the new best
            }
            if (right < count && items[right].comesBefore(items[best]))
            {                 // right child better?
                best = right; // right child is the new best
            }
            if (best == i)
            {          // already in the right spot?
                break; // stop moving down
            }
            swapItems(i, best); // swap with the better child
            i = best;           // continue from the child's spot
        }
    }

public:
    // Constructor: makes an empty heap
    EmailHeap()
    {
        items.reset(new Message[INITIAL_CAPACITY]); // allocate the starting array
        count = 0;                                  // no emails yet
        capacity = INITIAL_CAPACITY;                // remember the array size
    }

    // Copying is turned off so two heaps never share one array
    EmailHeap(const EmailHeap &) = delete;
    EmailHeap &operator=(const EmailHeap &) = delete;

    // Adds an email to the heap
    void insert(const Message &m)
    {
        if (count == capacity)
        {                     // array full?
            doubleCapacity(); // make it bigger first
        }
        items[count] = m; // put the new email at the end
        moveUp(count);    // move it up to its correct spot
        count++;          // one more email in the heap
    }

    // Returns true when there are no emails
    bool isEmpty() const { return count == 0; }

    // Returns how many emails are in the heap
    int size() const { return count; }

    // Returns the highest priority email (call isEmpty first)
    const Message &peekTop() const { return items[0]; }

    // Removes the highest priority email
    void removeTop()
    {
        if (count == 0)
        {           // nothing to remove
            return; // leave right away
        }
        items[0] = items[count - 1]; // move the last email to the top
        count--;                     // heap is one smaller
        if (count > 0)
        {                // anything left to fix?
            moveDown(0); // move the new top down to its spot
        }
    }
};

// ---------------------------------------------------------------------------
// InboxManager: reads commands and uses the heap to run them
// ---------------------------------------------------------------------------
class InboxManager
{
private:
    EmailHeap heap;    // the priority queue of unread emails
    long arrivalCount; // counts emails as they arrive (for tie-breaking)

    // Removes spaces, tabs, and \r (Windows line ending) from both ends of text
    string trim(const string &text) const
    {
        size_t first = text.find_first_not_of(" \t\r\n"); // first real character
        if (first == string::npos)
        {              // only whitespace?
            return ""; // give back an empty string
        }
        size_t last = text.find_last_not_of(" \t\r\n"); // last real character
        return text.substr(first, last - first + 1);    // keep what is in between
    }

    // Turns a category name into a rank (5 = Boss ... 1 = OtherPerson, 0 = invalid)
    int rankOfCategory(const string &category) const
    {
        if (category == "Boss")
            return 5; // Boss is read first
        if (category == "Subordinate")
            return 4; // Subordinate is next
        if (category == "Peer")
            return 3; // Peer is next
        if (category == "ImportantPerson")
            return 2; // ImportantPerson is next
        if (category == "OtherPerson")
            return 1; // OtherPerson is last
        return 0;     // not one of the five categories
    }

    // Checks a MM-DD-YYYY date and returns it as YYYYMMDD, or -1 if it is bad
    int convertDate(const string &date) const
    {
        if (date.length() != 10)
            return -1; // must be exactly 10 characters
        if (date[2] != '-' || date[5] != '-')
            return -1; // dashes must be in the right places
        for (int i = 0; i < 10; i++)
        { // look at every character
            if (i == 2 || i == 5)
                continue; // skip the two dashes
            if (date[i] < '0' || date[i] > '9')
                return -1; // everything else must be a digit
        }
        int month = (date[0] - '0') * 10 + (date[1] - '0');                                                 // build the month number
        int day = (date[3] - '0') * 10 + (date[4] - '0');                                                   // build the day number
        int year = (date[6] - '0') * 1000 + (date[7] - '0') * 100 + (date[8] - '0') * 10 + (date[9] - '0'); // build the year number
        if (month < 1 || month > 12)
            return -1; // month must be 1 to 12
        if (day < 1 || day > 31)
            return -1;                           // day must be 1 to 31
        return year * 10000 + month * 100 + day; // YYYYMMDD so bigger = newer
    }

    // Handles the text after "EMAIL " (category,subject,date)
    void addEmail(const string &fields)
    {
        size_t comma1 = fields.find(','); // first comma
        size_t comma2 = (comma1 == string::npos)
                            ? string::npos
                            : fields.find(',', comma1 + 1); // second comma
        if (comma1 == string::npos || comma2 == string::npos)
        { // missing a comma?
            cerr << "Warning: EMAIL needs category,subject,date. Skipped." << endl;
            return; // skip this email
        }
        string category = trim(fields.substr(0, comma1));                      // text before first comma
        string subject = trim(fields.substr(comma1 + 1, comma2 - comma1 - 1)); // text between commas
        string date = trim(fields.substr(comma2 + 1));                         // text after second comma

        int rank = rankOfCategory(category); // look up the category rank
        if (rank == 0)
        { // not a valid category?
            cerr << "Warning: unknown sender category \"" << category << "\". Skipped." << endl;
            return; // skip this email
        }
        if (subject.empty())
        { // blank subject?
            cerr << "Warning: empty subject line. Skipped." << endl;
            return; // skip this email
        }
        int dateValue = convertDate(date); // check and convert the date
        if (dateValue == -1)
        { // bad date?
            cerr << "Warning: bad date \"" << date << "\" (need MM-DD-YYYY). Skipped." << endl;
            return; // skip this email
        }

        Message m(category, subject, date, rank, dateValue, arrivalCount); // build the email
        arrivalCount++;                                                    // count this arrival
        heap.insert(m);                                                    // add it to the heap
    }

public:
    // Constructor: starts the arrival counter at 0
    InboxManager()
    {
        arrivalCount = 0; // no emails have arrived yet
    }

    // Runs one line from the input
    void runCommand(const string &rawLine)
    {
        string line = trim(rawLine); // clean up the line
        if (line.empty())
        {           // blank line?
            return; // nothing to do
        }

        if (line.compare(0, 6, "EMAIL ") == 0)
        {                             // line starts with "EMAIL "?
            addEmail(line.substr(6)); // pass along everything after it
        }
        else if (line == "COUNT")
        { // COUNT command
            cout << "There are " << heap.size() << " emails to read." << endl
                 << endl;
        }
        else if (line == "NEXT")
        { // NEXT command
            if (heap.isEmpty())
            { // inbox empty?
                cout << "No emails to read." << endl
                     << endl;
            }
            else
            {
                heap.peekTop().print(); // show top email, do not remove it
            }
        }
        else if (line == "READ")
        { // READ command
            if (heap.isEmpty())
            { // inbox empty?
                cout << "No emails to read." << endl
                     << endl;
            }
            else
            {
                heap.removeTop(); // remove the top email
            }
        }
        else
        { // anything else
            cerr << "Warning: unknown command \"" << line << "\". Skipped." << endl;
        }
    }

    // Reads every line from an input stream (a file or cin) and runs it
    void processStream(istream &input)
    {
        string line; // holds one line at a time
        while (getline(input, line))
        {                     // keep going until the input ends
            runCommand(line); // run that line
        }
    }
};

int main(int argc, char *argv[])
{
    InboxManager inbox; // the object that does all the work

    if (argc > 1)
    {                           // did the user give a file name?
        ifstream file(argv[1]); // try to open that file
        if (!file.is_open())
        { // could not open it?
            cerr << "Error: could not open file " << argv[1] << endl;
            return 1; // stop with an error code
        }
        inbox.processStream(file); // read commands from the file
    }
    else
    {                             // no file name given
        inbox.processStream(cin); // read from standard input (works with < file)
    }

    return 0; // finished with no errors
}