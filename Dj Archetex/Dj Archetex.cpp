/*
DJ Set Architect (ITCS 2530 Starter Project)
Author: Yousif Triki

Purpose (Original / Weeks 1-4):
- Store DJ tracks in a small library (array of structs)
- View a formatted table summary
- Recommend next tracks based on BPM + energy rules
- Save a report to a text file

Week 05 Upgrade (Ch. 12 focus):
- Abstract base class + virtual functions + polymorphism
- Composition (MixNotes inside derived track types)
- Manager class owns a dynamic array of base pointers (TrackBase**)
- Dynamic allocation for objects (new/delete), no STL containers
- Add/remove items, resize array, and clean up memory safely
- Updated doctests to validate polymorphism + manager behavior

Week 06 Upgrade (Ch. 13 focus: Operator Overloading + Templates):
- Operator overloading:
  * operator== for at least one derived class (meaningful identity match)
  * operator<< outputs one-line summary and uses polymorphism via virtual toStream()
  * TrackManager operator[] with bounds checking (no exceptions) [Week 06 version]
  * TrackManager operator+= / operator-= to add/remove items
  * Explicit use of this pointer
- Templates:
  * Function template used in the program
  * Class template replaces Week 05 dynamic array logic
- Doctests updated for:
  * == tests, << tests, [] tests, +=/-= tests, template function tests, class template tests

Week 07 Upgrade (Ch. 14 focus: Exception Handling):
- Requirements:
  * TrackManager operator[] must THROW on invalid index (negative or >= size)
  * TrackManager operator-= must THROW on invalid removal index
  * Template class DynamicArray must also THROW on invalid access/removal
  * At least one custom exception class derived from std::runtime_error used meaningfully
  * Update doctests for the new throwing behavior
  * Show CRT leak check output (Visual Studio / MSVC)

Week 09 Upgrade (Ch. 16 focus: Searching, Sorting, and the Vector Type):
- std::vector<int> bpmList added to TrackManager (replaces what would be a raw int array)
  * Uses .push_back(), .size(), .at() -- no raw array for this portion
  * Kept in sync with items: pushed on add(), erased on removeAt()
- Sequential (Linear) Search: sequentialSearchBpm()
  * Searches element-by-element from index 0; returns index or -1 if not found
  * No std::find or library search helpers used
- Bubble Sort: sortBpmsBubble()
  * Manually implemented; compares adjacent pairs and swaps until sorted ascending
  * No std::sort used
- Binary Search: binarySearchBpm()
  * Classic low/mid/high approach; requires sortBpmsBubble() called first
  * Returns index or -1; no std::binary_search used
- Doctests updated for all new functions including edge cases (empty vector, not found)

Week 10 Upgrade (Ch. 17 focus: Linked Lists):
- Replaced TrackManager BPM helper storage with a custom linked list ADT (no std::list)
- Added node structure (BpmNode) with data + next pointer
- Added linked list iterator class (BpmListIterator) for traversal
- Linked list operations implemented as member functions:
  * Insert (insertFront, insertBack)
  * Delete (removeFirstMatch, removeAtPosition helper)
  * Search (search)
  * Print/Traverse (print using iterator)
- Linked list type chosen: UNORDERED
  * Why: simple insertion behavior and clear demonstration of front/back insert positions

Week 11 Upgrade (Ch. 18 focus: Stacks and Queues):
- Added a custom array-based stack class (no std::stack)
  * Chosen because top push/pop operations stay simple and constant-time
  * Fixed capacity makes full-stack edge cases easy to test clearly
- Added a custom circular array queue class (no std::queue)
  * Chosen because front/back operations stay constant-time without shifting elements
- Updated TrackManager output to show:
  * Recent additions in stack (LIFO) order
  * Playback preview in queue (FIFO) order
- Added doctests for both ADTs and their integration output

Week 12 Upgrade (STL Maps focus):
- Added std::map<string, int> titleIndex to TrackManager
  * Maps track title keys to their current TrackManager array index
  * Supports insert on add, lookup by title, delete on remove, and iteration display
  * Enhances title lookup/removal without scanning the whole dynamic array

Concepts used (rubric):
- Constants (no magic numbers), enum, struct, array, classes
- Input validation (cin fail states, clear/ignore)
- Switch menu, compound if/else conditions
- Loops: for, while, do-while
- User-defined functions
- Formatted output with setw + precision
- File output with ofstream
- Inheritance, composition, virtual functions, abstract classes
- Dynamic memory with new/delete
- Operator overloading + templates (Ch 13)
- Exception handling + custom exception (Ch 14)
*/

#ifdef _MSC_VER
// -------------------- Week 07: CRT Leak Checking (MSVC) --------------------
// This makes Visual Studio print memory leak info when the program exits.
// In your Week 07 video, show the Output window result of this.
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#ifdef _DEBUG
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#endif

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <limits>
#include <sstream>
#include <stdexcept>   // runtime_error, out_of_range
#include <exception>
#include <cstdio>
#include <vector>      // Week 09 include kept for minimal change history
#include <map>         // Week 12: std::map title lookup index
#include "json.hpp"    // Week 13: nlohmann/json single-header library

using namespace std;
using nlohmann::json;

// -------------------- Constants (avoid magic numbers) --------------------
// Original (Weeks 1-4) library limits
const int MAX_TRACKS = 7;        // Maximum number of tracks stored

// Original table widths
const int TITLE_W = 22;         // Column widths for table formatting
const int ARTIST_W = 18;
const int GENRE_W = 12;
const int KEY_W = 6;
const int NOTE_W = 20;
const int LINE_W = 78;

// Week 5 display width for "Type" column
const int TYPE_W = 12;

// Validation ranges
const int BPM_MIN = 60;
const int BPM_MAX = 200;
const char* const DEFAULT_JSON_FILE = "dj_tracks.json";

// Menu range (merged menu: original + week5 + week9)
const int MENU_MIN = 1;
const int MENU_MAX = 14; // Week 12: expanded for std::map title lookup/removal

// -------------------- Enum --------------------
// EnergyLevel models how intense a track feels in a set (meaningful for DJ planning).
enum EnergyLevel { LOW = 1, MEDIUM = 2, HIGH = 3 };

// -------------------- Struct (Weeks 1-4) --------------------
// Track groups all track data together (meaningful model for the hobby).
struct Track
{
    string title;
    string artist;
    string genre;
    string key;
    int bpm = 0;                 // default
    EnergyLevel energy = MEDIUM; // default
    string notes;
};

// -------------------- Function Prototypes --------------------
// UI
void showBanner();
void showMenu();

// Input + validation
int getMenuChoice(int minChoice, int maxChoice);
string getNonEmptyLine(const string& prompt);
int getValidatedInt(const string& prompt, int minVal, int maxVal);
double getValidatedDouble(const string& prompt, double minVal, double maxVal);

// Enum helpers
EnergyLevel getEnergyFromUser();
string energyToString(EnergyLevel e);
EnergyLevel energyFromString(const string& energyText);

// -------------------- Weeks 1-4 Features --------------------
void addTrack(Track library[], int& count);
void printLibrary(const Track library[], int count);
void recommendNextTracks(const Track library[], int count);
void saveReportToFile(const Track library[], int count, const string& filename);

// Derived values / calculationsdat
double computeAverageBPM(const Track library[], int count);
int countGenreMatches(const Track library[], int count, const string& genre);

// Output helpers (Weeks 1-4)
void printLegacyTableHeader(ostream& out);
void printTrackRow(ostream& out, const Track& t);

// -------------------- Week 5/6/7 Helpers --------------------
int safeIndexFromUser(const string& prompt, int size);
void printWeek5TableHeader(ostream& out);
void printSeparator(ostream& out);

// -------------------- Week 06: Function Template --------------------
// REQUIREMENT: Create one function template and use it in your program.
//
// We use absValue<T> to compute absolute differences (ex: BPM difference).
// This is safer than repeating int-only logic and proves template usage.
template <class T>
T absValue(T x)
{
    return (x < static_cast<T>(0)) ? -x : x;
}

// -------------------- Week 07: Custom Exception --------------------
// REQUIREMENT: At least one custom exception class.
// We derive from std::runtime_error, store a message, and return it with what().
class DJException : public std::runtime_error
{
public:
    DJException(const std::string& msg)
        : std::runtime_error(msg) {}
};

// -------------------- Week 14: REST API Client --------------------
// Derived class inheriting from the provided HttpClient framework
class DjApiClient : public HttpClient
{
private:
    std::string responseBody;

protected:
    // Overrides StartOfData to clear out any previous accumulations
    void StartOfData() override
    {
        responseBody.clear();
    }

    // Overrides Data to build the raw HTTP response gradually
    void Data(const char* data, const unsigned int len) override
    {
        responseBody.append(data, len);
    }

    // Overrides EndOfData when reading is completed
    void EndOfData() override
    {
        // No strict finalization needed for strings
    }

public:
    DjApiClient() : responseBody("") {}

    // Exposes the completed payload safely 
    std::string GetResponse() const
    {
        return responseBody;
    }
};

#ifdef _MSC_VER
// Enable leak-check-at-exit automatically (useful for doctest runs too).
struct CrtLeakGuard
{
    CrtLeakGuard()
    {
        _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    }
};
static CrtLeakGuard g_crtLeakGuard;
#endif

// -------------------- Week 5: Composition Helper Class --------------------
// MixNotes is a small helper class used inside derived classes (composition).
class MixNotes
{
private:
    string notes;

public:
    MixNotes() : notes("") {}
    MixNotes(const string& n) : notes(n) {}

    void setNotes(const string& n) { notes = n; }
    string getNotes() const { return notes; }

    bool hasNotes() const
    {
        return !notes.empty();
    }
};

// -------------------- Week 06: Class Template (replaces Week 05 dynamic array logic) --------------------
// Week 07 change: now THROWS on invalid access/removal (Chapter 14 requirement).
template <class T>
class DynamicArray
{
private:
    T* items;
    int size;
    int capacity;

    void resize(int newCap)
    {
        T* newItems = new T[newCap];

        for (int i = 0; i < size; i++)
            newItems[i] = items[i];

        delete[] items;
        items = newItems;
        capacity = newCap;
    }

    DynamicArray(const DynamicArray&) = delete;
    DynamicArray& operator=(const DynamicArray&) = delete;

public:
    DynamicArray(int cap = 2)
        : items(nullptr), size(0), capacity(cap)
    {
        if (capacity < 2) capacity = 2;
        items = new T[capacity];
    }

    int getSize() const { return size; }
    int getCapacity() const { return capacity; }

    void pushBack(const T& value)
    {
        if (size >= capacity)
            resize(capacity * 2);

        items[size] = value;
        size++;
    }

    // Week 07 requirement: invalid removal -> throw
    void removeAt(int index)
    {
        if (index < 0 || index >= size)
            throw out_of_range("DynamicArray::removeAt invalid index");

        for (int i = index; i < size - 1; i++)
            items[i] = items[i + 1];

        size--;

        if (size > 0 && size <= capacity / 4 && capacity > 2)
            resize(capacity / 2);
    }

    // Week 07 requirement: invalid indexing -> throw
    T at(int index) const
    {
        if (index < 0 || index >= size)
            throw out_of_range("DynamicArray::at invalid index");
        return items[index];
    }

    // Non-throwing internal access used only when caller guarantees validity.
    T& rawAt(int index) { return items[index]; }
    const T& rawAt(int index) const { return items[index]; }

    ~DynamicArray()
    {
        delete[] items;
    }
};

// -------------------- Week 5/6/7: Abstract Base Class --------------------
// TrackBase is an ABSTRACT base class.
class TrackBase
{
protected:
    string title;  // protected for derived access

private:
    int bpm;
    EnergyLevel energy;

public:
    TrackBase() : title(""), bpm(0), energy(MEDIUM) {}

    TrackBase(const string& t, int b, EnergyLevel e)
        : title(t), bpm(b), energy(e) {
    }

    string getTitle() const { return title; }
    int getBpm() const { return bpm; }
    EnergyLevel getEnergy() const { return energy; }

    void setTitle(const string& t) { title = t; }
    void setBpm(int b) { bpm = b; }
    void setEnergy(EnergyLevel e) { energy = e; }

    virtual void print(ostream& out) const
    {
        out << left
            << setw(TITLE_W) << title.substr(0, TITLE_W - 1)
            << setw(TYPE_W) << getType()
            << right << setw(6) << bpm << "  "
            << left << setw(8) << energyToString(energy);
    }

    // Week 06: used by operator<< (polymorphic one-line)
    virtual void toStream(ostream& out) const
    {
        out << getType() << " | " << title << " | " << bpm << " BPM | " << energyToString(energy);
    }

    virtual string getType() const = 0;

    virtual ~TrackBase() {}
};

// -------------------- Week 06: operator<< overload (polymorphic) --------------------
ostream& operator<<(ostream& out, const TrackBase& t)
{
    t.toStream(out);  // virtual call => derived override runs
    return out;
}

// -------------------- Week 5/6/7: Derived Class #1 --------------------
class LocalTrack : public TrackBase
{
private:
    string filePath;
    MixNotes notes; // composition

public:
    LocalTrack() : TrackBase(), filePath(""), notes() {}

    LocalTrack(const string& t, int b, EnergyLevel e,
        const string& path, const MixNotes& n)
        : TrackBase(t, b, e), filePath(path), notes(n) {
    }

    void setFilePath(const string& p) { filePath = p; }
    string getFilePath() const { return filePath; }

    void setNotes(const MixNotes& n) { notes = n; }
    MixNotes getNotes() const { return notes; }

    string getType() const override { return "LocalTrack"; }

    void print(ostream& out) const override
    {
        TrackBase::print(out);
        out << setw(KEY_W) << ""
            << setw(NOTE_W) << (notes.hasNotes() ? notes.getNotes().substr(0, NOTE_W - 1) : "(none)")
            << "  Path: " << filePath;
    }

    void toStream(ostream& out) const override
    {
        out << getType()
            << " | " << title
            << " | " << getBpm() << " BPM"
            << " | " << energyToString(getEnergy())
            << " | Path=" << filePath;
    }

    // Week 06 requirement: operator== for at least one derived class
    // Identity choice: title + filePath uniquely identify local track in this program.
    bool operator==(const LocalTrack& other) const
    {
        return (this->title == other.title) && (this->filePath == other.filePath);
    }
};

// -------------------- Week 5/6/7: Derived Class #2 --------------------
class StreamTrack : public TrackBase
{
private:
    string platform;
    MixNotes notes; // composition

public:
    StreamTrack() : TrackBase(), platform(""), notes() {}

    StreamTrack(const string& t, int b, EnergyLevel e,
        const string& plat, const MixNotes& n)
        : TrackBase(t, b, e), platform(plat), notes(n) {
    }

    void setPlatform(const string& p) { platform = p; }
    string getPlatform() const { return platform; }

    void setNotes(const MixNotes& n) { notes = n; }
    MixNotes getNotes() const { return notes; }

    string getType() const override { return "StreamTrack"; }

    void print(ostream& out) const override
    {
        TrackBase::print(out);
        out << setw(KEY_W) << ""
            << setw(NOTE_W) << (notes.hasNotes() ? notes.getNotes().substr(0, NOTE_W - 1) : "(none)")
            << "  Platform: " << platform;
    }

    void toStream(ostream& out) const override
    {
        out << getType()
            << " | " << title
            << " | " << getBpm() << " BPM"
            << " | " << energyToString(getEnergy())
            << " | Platform=" << platform;
    }
};

// -------------------- Week 10: Linked List Node --------------------
// Node stores one BPM value and a pointer to the next node.
struct BpmNode
{
    int data;
    BpmNode* next;

    BpmNode(int d) : data(d), next(nullptr) {}
};

// -------------------- Week 10: Linked List Iterator --------------------
// Supports:
// 1) initialization to front, 2) advancing, 3) reading current data.
class BpmListIterator
{
private:
    BpmNode* current;

public:
    BpmListIterator(BpmNode* start = nullptr) : current(start) {}

    bool isValid() const { return current != nullptr; }

    void next()
    {
        if (current != nullptr)
            current = current->next;
    }

    int data() const
    {
        if (current == nullptr)
            throw out_of_range("BpmListIterator::data invalid current node");
        return current->data;
    }
};

// -------------------- Week 10: Unordered Linked List ADT --------------------
// Why unordered?
// - We can demonstrate at least two insertion positions (front and back)
//   in a simple way without enforcing automatic sorted insert behavior.
class BpmLinkedList
{
private:
    BpmNode* head;
    BpmNode* tail;
    int listSize;

    BpmLinkedList(const BpmLinkedList&) = delete;
    BpmLinkedList& operator=(const BpmLinkedList&) = delete;

public:
    BpmLinkedList() : head(nullptr), tail(nullptr), listSize(0) {}

    ~BpmLinkedList()
    {
        clear();
    }

    void clear()
    {
        BpmNode* cur = head;
        while (cur != nullptr)
        {
            BpmNode* nextNode = cur->next;
            delete cur;
            cur = nextNode;
        }
        head = nullptr;
        tail = nullptr;
        listSize = 0;
    }

    int getSize() const { return listSize; }

    // Insert position #1: front
    void insertFront(int value)
    {
        BpmNode* n = new BpmNode(value);
        n->next = head;
        head = n;
        if (tail == nullptr)
            tail = n;
        listSize++;
    }

    // Insert position #2: back
    void insertBack(int value)
    {
        BpmNode* n = new BpmNode(value);
        if (tail == nullptr)
        {
            head = n;
            tail = n;
        }
        else
        {
            tail->next = n;
            tail = n;
        }
        listSize++;
    }

    // Delete operation: remove first node matching value.
    bool removeFirstMatch(int value)
    {
        BpmNode* prev = nullptr;
        BpmNode* cur = head;

        while (cur != nullptr)
        {
            if (cur->data == value)
            {
                if (prev == nullptr)
                    head = cur->next;
                else
                    prev->next = cur->next;

                if (cur == tail)
                    tail = prev;

                delete cur;
                listSize--;
                return true;
            }

            prev = cur;
            cur = cur->next;
        }

        return false;
    }

    // Helper used by TrackManager to keep index alignment with items[].
    bool removeAtPosition(int index)
    {
        if (index < 0 || index >= listSize)
            return false;

        BpmNode* prev = nullptr;
        BpmNode* cur = head;

        for (int i = 0; i < index; i++)
        {
            prev = cur;
            cur = cur->next;
        }

        if (prev == nullptr)
            head = cur->next;
        else
            prev->next = cur->next;

        if (cur == tail)
            tail = prev;

        delete cur;
        listSize--;
        return true;
    }

    // Search operation: sequentially return index of first match or -1.
    int search(int value) const
    {
        int index = 0;
        BpmNode* cur = head;

        while (cur != nullptr)
        {
            if (cur->data == value)
                return index;
            cur = cur->next;
            index++;
        }

        return -1;
    }

    // Access node data at index (throws on invalid index).
    int at(int index) const
    {
        if (index < 0 || index >= listSize)
            throw out_of_range("BpmLinkedList::at invalid index");

        BpmNode* cur = head;
        for (int i = 0; i < index; i++)
            cur = cur->next;

        return cur->data;
    }

    // Set node data at index (throws on invalid index).
    void setAt(int index, int value)
    {
        if (index < 0 || index >= listSize)
            throw out_of_range("BpmLinkedList::setAt invalid index");

        BpmNode* cur = head;
        for (int i = 0; i < index; i++)
            cur = cur->next;

        cur->data = value;
    }

    BpmListIterator begin() const
    {
        return BpmListIterator(head);
    }

    // Print/Traverse operation (uses iterator per Week 10 requirement).
    void print(ostream& out) const
    {
        BpmListIterator it = begin();
        if (!it.isValid())
        {
            out << "(empty)\n";
            return;
        }

        while (it.isValid())
        {
            out << it.data() << " ";
            it.next();
        }
        out << "\n";
    }
};

// -------------------- Week 11: Array-Based Stack ADT --------------------
// Choice: array-based stack
// Why: push/pop/top are straightforward O(1) operations, and a fixed capacity
// makes it easy to demonstrate the "full stack" edge case required by the assignment.
template <class T>
class ArrayStack
{
private:
    T* items;
    int capacity;
    int topIndex;

    ArrayStack(const ArrayStack&) = delete;
    ArrayStack& operator=(const ArrayStack&) = delete;

public:
    ArrayStack(int cap = MAX_TRACKS)
        : items(nullptr), capacity(cap), topIndex(-1)
    {
        if (capacity < 1)
            capacity = 1;

        items = new T[capacity];
    }

    ~ArrayStack()
    {
        delete[] items;
    }

    bool isEmpty() const
    {
        return topIndex < 0;
    }

    bool isFull() const
    {
        return topIndex + 1 >= capacity;
    }

    int getSize() const
    {
        return topIndex + 1;
    }

    void push(const T& value)
    {
        if (isFull())
            throw DJException("ArrayStack::push full stack");

        topIndex++;
        items[topIndex] = value;
    }

    void pop()
    {
        if (isEmpty())
            throw DJException("ArrayStack::pop empty stack");

        topIndex--;
    }

    T top() const
    {
        if (isEmpty())
            throw DJException("ArrayStack::top empty stack");

        return items[topIndex];
    }
};

// -------------------- Week 11: Circular Array Queue ADT --------------------
// Choice: array-based circular queue
// Why: enqueue/dequeue/front remain O(1) and the circular indexing avoids shifting.
template <class T>
class ArrayQueue
{
private:
    T* items;
    int capacity;
    int frontIndex;
    int backIndex;
    int count;

    ArrayQueue(const ArrayQueue&) = delete;
    ArrayQueue& operator=(const ArrayQueue&) = delete;

public:
    ArrayQueue(int cap = MAX_TRACKS)
        : items(nullptr), capacity(cap), frontIndex(0), backIndex(0), count(0)
    {
        if (capacity < 1)
            capacity = 1;

        items = new T[capacity];
    }

    ~ArrayQueue()
    {
        delete[] items;
    }

    bool isEmpty() const
    {
        return count == 0;
    }

    bool isFull() const
    {
        return count >= capacity;
    }

    int getSize() const
    {
        return count;
    }

    void enqueue(const T& value)
    {
        if (isFull())
            throw DJException("ArrayQueue::enqueue full queue");

        items[backIndex] = value;
        backIndex = (backIndex + 1) % capacity;
        count++;
    }

    void dequeue()
    {
        if (isEmpty())
            throw DJException("ArrayQueue::dequeue empty queue");

        frontIndex = (frontIndex + 1) % capacity;
        count--;
    }

    T front() const
    {
        if (isEmpty())
            throw DJException("ArrayQueue::front empty queue");

        return items[frontIndex];
    }
};

// -------------------- Week 5/6/7/9/10: Manager Class --------------------
// TrackManager OWNS TrackBase* objects and uses DynamicArray<TrackBase*> for storage.
class TrackManager
{
private:
    DynamicArray<TrackBase*> items; // template-based dynamic container

    // -------------------- Week 10: replace BPM helper storage with linked list --------------------
    // This linked list stays index-aligned with items[].
    BpmLinkedList bpmList;

    // -------------------- Week 12: STL map title lookup index --------------------
    // std::map is used here instead of repeatedly scanning DynamicArray because a
    // title is a meaningful key and map::find gives direct key-based lookup.
    // The DynamicArray still owns the TrackBase objects; this map stores only
    // each title key and the current array index for that track.
    map<string, int> titleIndex;

    void rebuildTitleIndex()
    {
        titleIndex.clear();

        for (int i = 0; i < items.getSize(); i++)
        {
            TrackBase* p = items.rawAt(i);
            if (p != nullptr)
                titleIndex[p->getTitle()] = i;
        }
    }

    int countHighEnergyRecursiveHelper(int index) const
    {
        // Base case: reached end of array
        if (index >= items.getSize())
            return 0;

        // Recursive case: count current item if HIGH, then move to next
        int currentIsHigh = 0;
        if (items.rawAt(index) != nullptr && items.rawAt(index)->getEnergy() == HIGH)
            currentIsHigh = 1;

        return currentIsHigh + countHighEnergyRecursiveHelper(index + 1);
    }

    TrackManager(const TrackManager&) = delete;
    TrackManager& operator=(const TrackManager&) = delete;

public:
    TrackManager(int cap = 2)
        : items(cap)
    {
    }

    int getSize() const { return items.getSize(); }
    int getCapacity() const { return items.getCapacity(); }

    int countHighEnergyRecursive() const
    {
        return countHighEnergyRecursiveHelper(0);
    }

    // Adds a pointer (manager takes ownership)
    void add(TrackBase* p)
    {
        int newIndex = items.getSize();
        items.pushBack(p);
        bpmList.insertBack(p->getBpm()); // Week 10: insert BPM into linked list (back position)
        titleIndex[p->getTitle()] = newIndex; // Week 12: insert/update title -> index map pair
    }

    // Removes by index (deletes object, shifts close the gap)
    // Internal helper. We keep it throwing to match Week 07 behavior.
    void removeAt(int index)
    {
        // If invalid, DynamicArray::at throws out_of_range.
        TrackBase* doomed = items.at(index);
        string removedTitle = doomed->getTitle();
        titleIndex.erase(removedTitle); // Week 12: delete title key/value from the map
        delete doomed;
        items.removeAt(index);
        bpmList.removeAtPosition(index); // Week 10: keep linked list in sync with items
        rebuildTitleIndex();             // Week 12: shifted array indexes must stay accurate
    }

    // Week 07 requirement: operator[] must THROW on invalid index.
    // Also: we use our CUSTOM exception here (DJException) to satisfy the rubric.
    TrackBase* operator[](int index) const
    {
        if (index < 0 || index >= items.getSize())
            throw DJException("TrackManager::operator[] invalid index");

        // Valid index, safe to access:
        return items.at(index);  // will not throw here because we checked
    }

    // Week 06: operator+= adds item pointer to container
    TrackManager& operator+=(TrackBase* p)
    {
        // explicit use of this pointer (Week 06 requirement)
        this->add(p);
        return *this;
    }

    // Week 07 requirement: operator-= must THROW on invalid removal.
    TrackManager& operator-=(int index)
    {
        if (index < 0 || index >= items.getSize())
            throw DJException("TrackManager::operator-= invalid removal index");

        this->removeAt(index);
        return *this;
    }

    void printAll(ostream& out) const
    {
        if (items.getSize() == 0)
        {
            out << "No tracks stored yet.\n";
            return;
        }

        printWeek5TableHeader(out);

        for (int i = 0; i < items.getSize(); i++)
        {
            out << setw(4) << i << " ";
            TrackBase* p = items.rawAt(i); // valid i, so rawAt is safe (no exception)
            if (p)
                p->print(out);
            out << "\n";
        }

        printSeparator(out);
    }

    const TrackBase* findTrackByTitle(const string& title) const
    {
        map<string, int>::const_iterator it = titleIndex.find(title); // Week 12 lookup
        if (it == titleIndex.end())
            return nullptr;

        int index = it->second;
        if (index < 0 || index >= items.getSize())
            return nullptr;

        return items.rawAt(index);
    }

    bool removeTrackByTitle(const string& title)
    {
        map<string, int>::const_iterator it = titleIndex.find(title); // Week 12 lookup before delete
        if (it == titleIndex.end())
            return false;

        int index = it->second;
        removeAt(index); // removeAt deletes the owned object and erases the map entry
        return true;
    }

    int getTitleMapCount() const
    {
        return static_cast<int>(titleIndex.size());
    }

    void printTitleMap(ostream& out) const
    {
        out << "Title map lookup (std::map): ";

        if (titleIndex.empty())
        {
            out << "(empty)\n";
            return;
        }

        bool first = true;
        for (map<string, int>::const_iterator it = titleIndex.begin(); it != titleIndex.end(); ++it)
        {
            if (!first)
                out << " | ";

            out << it->first << " => index " << it->second;
            first = false;
        }

        out << "\n";
    }

    void saveReport(const string& filename) const
    {
        ofstream fout(filename.c_str());
        if (!fout)
        {
            cout << "Could not open file: " << filename << "\n";
            return;
        }

        fout << "==================== DJ SET ARCHITECT REPORT (Week 7) ====================\n";
        fout << "Tracks stored: " << items.getSize() << "\n\n";

        printAll(fout);

        fout.close();
        cout << "Report saved to " << filename << "\n";
    }

    // -------------------- Week 09: Sequential (Linear) Search --------------------
    // Scans bpmList element-by-element from the start.
    // Returns the index of the first match, or -1 if the target BPM is not found.
    // No std::find or any library search is used — the loop does all the work.
    int sequentialSearchBpm(int target) const
    {
        return bpmList.search(target);
    }

    // -------------------- Week 09: Bubble Sort --------------------
    // Sorts bpmList in ascending order using the Bubble Sort algorithm.
    // Each outer pass "bubbles" the largest unsorted value to its final position.
    // No std::sort — every swap is done manually.
    void sortBpmsBubble()
    {
        int n = bpmList.getSize();
        for (int i = 0; i < n - 1; i++)
        {
            for (int j = 0; j < n - i - 1; j++)
            {
                if (bpmList.at(j) > bpmList.at(j + 1))
                {
                    // swap adjacent elements
                    int temp = bpmList.at(j);
                    bpmList.setAt(j, bpmList.at(j + 1));
                    bpmList.setAt(j + 1, temp);
                }
            }
        }
    }

    // -------------------- Week 09: Binary Search --------------------
    // IMPORTANT: sortBpmsBubble() must be called before this function.
    // Binary search requires the data to be in sorted order; without it the
    // mid-point comparison sends the search in the wrong direction.
    // Uses the classic low / mid / high index approach.
    // Returns the index of the found element, or -1 if not found.
    int binarySearchBpm(int target) const
    {
        int low  = 0;
        int high = bpmList.getSize() - 1;

        while (low <= high)
        {
            int mid = low + (high - low) / 2; // avoids integer overflow vs (low+high)/2

            if (bpmList.at(mid) == target)
                return mid;                   // exact match found
            else if (bpmList.at(mid) < target)
                low = mid + 1;                // target is in right half
            else
                high = mid - 1;              // target is in left half
        }
        return -1; // not found
    }

    // Week 09/10 helper: returns how many BPM values are stored
    int getBpmCount() const { return bpmList.getSize(); }

    // Week 09/10 helper: returns BPM value at position i
    int getBpmAt(int i) const { return bpmList.at(i); }

    bool loadTracksFromJsonFile(const string& filename, string& statusMessage)
    {
        try
        {
            ifstream fin(filename.c_str());
            if (!fin)
                throw DJException("Could not open JSON file: " + filename);

            json trackData;
            fin >> trackData;

            if (!trackData.is_array())
                throw DJException("JSON root must be an array of track objects.");

            int loadedCount = 0;

            // JSON data is converted into TrackBase-derived objects so it feeds the
            // existing manager, linked list, stack/queue views, and title map.
            for (json::const_iterator it = trackData.begin(); it != trackData.end(); ++it)
            {
                const json& entry = *it;

                const string type = entry.at("type").get<string>();
                const string title = entry.at("title").get<string>();
                const int bpm = entry.at("bpm").get<int>();
                const string energyText = entry.at("energy").get<string>();
                const string source = entry.at("source").get<string>();
                const string notes = entry.at("notes").get<string>();

                if (bpm < BPM_MIN || bpm > BPM_MAX)
                    throw DJException("JSON BPM out of range for track: " + title);

                const EnergyLevel energy = energyFromString(energyText);

                if (type == "local")
                    add(new LocalTrack(title, bpm, energy, source, MixNotes(notes)));
                else if (type == "stream")
                    add(new StreamTrack(title, bpm, energy, source, MixNotes(notes)));
                else
                    throw DJException("Unsupported track type in JSON: " + type);

                loadedCount++;
            }

            ostringstream oss;
            oss << "Loaded " << loadedCount << " tracks from " << filename << ".";
            statusMessage = oss.str();
            return true;
        }
        catch (const exception& ex)
        {
            statusMessage = ex.what();
            return false;
        }
    }

    // Week 10 helper: linked list print/traverse in one place
    void printBpmLinkedList(ostream& out) const
    {
        out << "BPM linked list traversal: ";
        bpmList.print(out); // uses iterator internally
    }

    void printRecentAdditionsStack(ostream& out) const
    {
        out << "Recent additions stack (LIFO): ";

        if (items.getSize() == 0)
        {
            out << "(empty)\n";
            return;
        }

        ArrayStack<string> recentTitles(items.getSize());
        for (int i = 0; i < items.getSize(); i++)
            recentTitles.push(items.rawAt(i)->getTitle());

        while (!recentTitles.isEmpty())
        {
            out << recentTitles.top();
            recentTitles.pop();

            if (!recentTitles.isEmpty())
                out << " -> ";
        }

        out << "\n";
    }

    void printPlaybackQueue(ostream& out) const
    {
        out << "Playback queue (FIFO): ";

        if (items.getSize() == 0)
        {
            out << "(empty)\n";
            return;
        }

        ArrayQueue<string> playbackQueue(items.getSize());
        for (int i = 0; i < items.getSize(); i++)
            playbackQueue.enqueue(items.rawAt(i)->getTitle());

        while (!playbackQueue.isEmpty())
        {
            out << playbackQueue.front();
            playbackQueue.dequeue();

            if (!playbackQueue.isEmpty())
                out << " -> ";
        }

        out << "\n";
    }

    ~TrackManager()
    {
        // delete all owned objects
        for (int i = 0; i < items.getSize(); i++)
        {
            TrackBase* p = items.rawAt(i);
            delete p;
        }
        // DynamicArray destructor cleans up its internal array
    }
};

// -------------------- Main --------------------
#ifndef _DEBUG
int main()
{
#ifdef _MSC_VER
    // Ensure leak-check is enabled (also enabled via CrtLeakGuard above).
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    // Weeks 1-4 storage
    Track library[MAX_TRACKS];
    int trackCount = 0;

    // Week 5/6/7 storage
    TrackManager manager(2);
    string jsonStatus;

    showBanner();

    if (manager.loadTracksFromJsonFile(DEFAULT_JSON_FILE, jsonStatus))
        cout << "Week 13 JSON load: " << jsonStatus << "\n";
    else
        cout << "Week 13 JSON load skipped: " << jsonStatus << "\n";

    // 3+ mixed inputs: string (spaces), int, double (legacy)
    string djName = getNonEmptyLine("Enter your DJ name: ");
    int targetBPM = getValidatedInt("Enter target BPM for your set (60-200): ", BPM_MIN, BPM_MAX);
    double prepHours = getValidatedDouble("How many hours can you prep today (0.0 - 12.0)? ", 0.0, 12.0);

    // Compound if/else block #1 (rubric requirement)
    if (prepHours >= 4.0 && prepHours <= 8.0)
        cout << "\nNice. With " << prepHours << " hours, you can build a solid set.\n";
    else if (prepHours < 4.0 && targetBPM >= 125)
        cout << "\nShort prep time + high BPM target. Keep transitions simple.\n";
    else
        cout << "\nPlan smart: focus on clean BPM ranges and energy flow.\n";

    // Compound if/else block #2 (rubric requirement)
    if (targetBPM >= 128 && prepHours >= 5.0)
        cout << "Recommendation: build an energy climb into a peak-hour section.\n";
    else
        cout << "Recommendation: keep a steady groove and avoid risky key jumps.\n";

    // Fixed-count for loop (rubric requirement)
    cout << "\nQuick Set-Prep Tips:\n";
    const int TIP_COUNT = 3;
    for (int i = 1; i <= TIP_COUNT; i++)
    {
        if (i == 1) cout << "  1) Group tracks by BPM buckets.\n";
        if (i == 2) cout << "  2) Keep keys compatible when possible.\n";
        if (i == 3) cout << "  3) Increase energy gradually.\n";
    }

    int choice = 0;
    do
    {
        showMenu();
        choice = getMenuChoice(MENU_MIN, MENU_MAX);

        switch (choice)
        {
        case 1:
            addTrack(library, trackCount);
            break;
        case 2:
            printLibrary(library, trackCount);
            break;
        case 3:
            recommendNextTracks(library, trackCount);
            break;
        case 4:
            saveReportToFile(library, trackCount, "DJ_Set_Report.txt");
            break;

        case 5:
        {
            cout << "\n--- Add Local Track (Week 7) ---\n";
            string t = getNonEmptyLine("Title: ");
            int bpm = getValidatedInt("BPM (60-200): ", BPM_MIN, BPM_MAX);
            EnergyLevel e = getEnergyFromUser();
            string path = getNonEmptyLine("File path (ex: track.wav): ");
            string noteText = getNonEmptyLine("Notes (mix notes): ");

            manager += new LocalTrack(t, bpm, e, path, MixNotes(noteText));
            cout << "Local track added (Week 7).\n";
            break;
        }
        case 6:
        {
            cout << "\n--- Add Stream Track (Week 7) ---\n";
            string t = getNonEmptyLine("Title: ");
            int bpm = getValidatedInt("BPM (60-200): ", BPM_MIN, BPM_MAX);
            EnergyLevel e = getEnergyFromUser();
            string platform = getNonEmptyLine("Platform (ex: Spotify): ");
            string noteText = getNonEmptyLine("Notes (mix notes): ");

            manager += new StreamTrack(t, bpm, e, platform, MixNotes(noteText));
            cout << "Stream track added (Week 7).\n";
            break;
        }
        case 7:
            cout << "\n==================== WEEK 7 LIBRARY ====================\n";
            manager.printAll(cout);
            // NEW Week 08 recursive feature
            cout << "High-energy tracks (recursive): "
                << manager.countHighEnergyRecursive() << "\n";
            // Week 10: linked list traversal display (uses iterator)
            manager.printBpmLinkedList(cout);
            // Week 11: stack + queue display
            manager.printRecentAdditionsStack(cout);
            manager.printPlaybackQueue(cout);
            // Week 12: iterate all std::map key/value pairs
            manager.printTitleMap(cout);

            // operator[] now throws if invalid, so we only do it if size > 0
            if (manager.getSize() > 0)
            {
                try
                {
                    cout << "One-line summary (operator<<): " << *manager[0] << "\n";
                }
                catch (const exception& ex)
                {
                    cout << "Unexpected error: " << ex.what() << "\n";
                }
            }
            break;

        case 8:
        {
            cout << "\n--- Remove Week 7 Track ---\n";
            if (manager.getSize() == 0)
            {
                cout << "Nothing to remove.\n";
                break;
            }

            manager.printAll(cout);
            int idx = safeIndexFromUser("Enter index to remove: ", manager.getSize());

            // operator-= now throws if invalid removal
            try
            {
                manager -= idx;
                cout << "Removed item " << idx << ".\n";
            }
            catch (const exception& ex)
            {
                cout << "Remove failed: " << ex.what() << "\n";
            }

            break;
        }
        case 9:
            manager.saveReport("DJ_Set_Report_Week7.txt");
            break;

        // -------------------- Week 09: Sequential Search --------------------
        case 10:
        {
            if (manager.getBpmCount() == 0)
            {
                cout << "No tracks in Week 09/10 library yet. Add some first (options 5 or 6).\n";
                break;
            }
            int target = getValidatedInt("Enter BPM to search for (60-200): ", BPM_MIN, BPM_MAX);
            int idx = manager.sequentialSearchBpm(target);
            if (idx == -1)
                cout << "BPM " << target << " not found (sequential search).\n";
            else
                cout << "BPM " << target << " found at index " << idx
                     << " (sequential search): " << *manager[idx] << "\n";
            break;
        }

        // -------------------- Week 09: Bubble Sort + Binary Search --------------------
        case 11:
        {
            if (manager.getBpmCount() == 0)
            {
                cout << "No tracks in Week 09/10 library yet. Add some first (options 5 or 6).\n";
                break;
            }
            // Sort must come before binary search
            manager.sortBpmsBubble();
            cout << "BPM list sorted (bubble sort).\n";
            manager.printBpmLinkedList(cout);

            int target = getValidatedInt("Enter BPM to binary search for (60-200): ", BPM_MIN, BPM_MAX);
            int idx = manager.binarySearchBpm(target);
            if (idx == -1)
                cout << "BPM " << target << " not found (binary search).\n";
            else
                cout << "BPM " << target << " found at sorted index " << idx << ".\n";
            break;
        }

        case 12:
        {
            cout << "\n--- Lookup Track by Title (Week 12 std::map) ---\n";
            string title = getNonEmptyLine("Title to look up: ");
            const TrackBase* found = manager.findTrackByTitle(title);

            if (found == nullptr)
                cout << "Title \"" << title << "\" was not found in the map.\n";
            else
                cout << "Found by title map: " << *found << "\n";

            break;
        }

        case 13:
        {
            cout << "\n--- Remove Track by Title (Week 12 std::map) ---\n";
            if (manager.getTitleMapCount() == 0)
            {
                cout << "No mapped tracks to remove.\n";
                break;
            }

            manager.printTitleMap(cout);
            string title = getNonEmptyLine("Title to remove: ");

            if (manager.removeTrackByTitle(title))
                cout << "Removed \"" << title << "\" using the title map.\n";
            else
                cout << "Title \"" << title << "\" was not found in the map.\n";

            break;
        }

        // -------------------- Week 14: REST API GET --------------------
        case 14:
        {
            cout << "\n--- Fetch DJ Jokes (API GET) ---\n";
            DjApiClient apiClient;
            
            // Connect to server (HTTP not HTTPS as per assignment)
            if (!apiClient.Connect("api.macomb.io", 80))
            {
                cout << "Failed to connect to API server.\n";
                break;
            }

            string countStr = getNonEmptyLine("How many jokes do you want to fetch? (1-5): ");
            // Use AddQueryParameters via std::map
            map<string, string> qp;
            qp["count"] = countStr;
            cout << "Requesting from api.macomb.io/jokes...\n";
            
            try
            {
                if (apiClient.Get("/jokes", qp))
                {
                    string response = apiClient.GetResponse();
                    
                    // Parse JSON array and use the try-catch for exception handling
                    json j = json::parse(response);
                    if (j.is_array())
                    {
                        int added = 0;
                        for (json::iterator it = j.begin(); it != j.end(); ++it)
                        {
                            if (it->contains("joke"))
                            {
                                if (!apiJokesQueue.isFull())
                                {
                                    apiJokesQueue.enqueue((*it)["joke"].get<string>());
                                    added++;
                                }
                                else
                                {
                                    cout << "Joke queue is full! Not all jokes were added.\n";
                                    break;
                                }
                            }
                        }
                        cout << "Successfully added " << added << " jokes to the queue.\n";
                    }
                    else
                    {
                        cout << "Unexpected JSON format from API.\n";
                    }
                }
                else 
                {
                    cout << "API GET request failed.\n";
                }
            }
            catch (const nlohmann::json::exception& ex)
            {
                cout << "JSON Parsing Error (GET): " << ex.what() << "\n";
            }
            catch (const exception& ex)
            {
                cout << "HTTP Request Error: " << ex.what() << "\n";
            }
            break;
        }

        // -------------------- Week 14: REST API POST --------------------
        case 15:
        {
            cout << "\n--- Post a DJ Joke (API POST) ---\n";
            string myJoke = getNonEmptyLine("Enter your DJ Joke to post: ");
            
            // Build JSON object 
            json postBody;
            postBody["joke"] = myJoke;
            string jsonBody = postBody.dump();
            
            DjApiClient apiClient;
            if (!apiClient.Connect("api.macomb.io", 80))
            {
                cout << "Failed to connect to API server.\n";
                break;
            }

            try
            {
                cout << "Sending POST to http://api.macomb.io/jokes...\n";
                if (apiClient.Post("/jokes", jsonBody))
                {
                    string response = apiClient.GetResponse();
                    
                    // Parse the response to confirm assignment
                    json j = json::parse(response);
                    if (j.contains("id"))
                    {
                        cout << "Joke successfully posted! Assigned ID: " << j["id"] << "\n";
                    }
                    else
                    {
                        cout << "Joke posted, but no ID was returned. Raw Response: " << response << "\n";
                    }
                }
                else
                {
                    cout << "API POST request failed.\n";
                }
            }
            catch (const nlohmann::json::exception& ex)
            {
                cout << "JSON Parsing Error (POST): " << ex.what() << "\n";
            }
            catch (const exception& ex)
            {
                cout << "HTTP Request Error: " << ex.what() << "\n";
            }
            break;
        }

        // -------------------- Week 14: Meaningful Data Use --------------------
        case 16:
        {
            cout << "\n--- Hear DJ Jokes (Queue) ---\n";
            if (apiJokesQueue.isEmpty())
            {
                cout << "The joke queue is empty. Fetch some first (Option 14)!\n";
            }
            else
            {
                cout << "Next Joke in Queue: " << apiJokesQueue.front() << "\n";
                apiJokesQueue.dequeue();
            }
            break;
        }

        case 17:
            cout << "\nGoodbye, " << djName << "! Keep the crowd moving.\n";
            break;

        default:
            cout << "Invalid choice.\n";
        }

    } while (choice != 17);

    return 0;
}
#endif

// -------------------- UI Functions --------------------
void showBanner()
{
    cout << "=============================================\n";
    cout << "        DJ SET ARCHITECT - C++ EDITION       \n";
    cout << "Weeks 1-4 + Weeks 5/6/7/09/10/11/12 Upgrade Combined\n";
    cout << "=============================================\n";
}

void showMenu()
{
    cout << "\n-------------------- MENU --------------------\n";
    cout << "WEEKS 1-4 (Struct + Array)\n";
    cout << "1) Add a track to library\n";
    cout << "2) View library summary\n";
    cout << "3) Recommend next tracks (BPM/Energy rules)\n";
    cout << "4) Save report to file\n\n";

    cout << "WEEK 5+ (Abstract + Polymorphism + Operators + Templates + Exceptions)\n";
    cout << "5) Add Local Track\n";
    cout << "6) Add Stream Track\n";
    cout << "7) View library\n";
    cout << "8) Remove track by index\n";
    cout << "9) Save report to file\n\n";

    cout << "WEEK 09/10/11 (Search + Sort + Linked List + Stack + Queue)\n";
    cout << "10) Sequential search BPM in library\n";
    cout << "11) Sort library BPMs then binary search\n\n";

    cout << "WEEK 12 (STL map title index)\n";
    cout << "12) Lookup track by title\n";
    cout << "13) Remove track by title\n\n";

    cout << "WEEK 14 (REST API & JSON Parse)\n";
    cout << "14) Fetch DJ Jokes (API GET)\n";
    cout << "15) Post a DJ Joke (API POST)\n";
    cout << "16) Hear DJ Jokes (Queue)\n\n";

    cout << "17) Quit\n";
    cout << "----------------------------------------------\n";
}

int getMenuChoice(int minChoice, int maxChoice)
{
    int choice;
    while (true)
    {
        cout << "Enter choice (" << minChoice << "-" << maxChoice << "): ";

        if (cin >> choice && choice >= minChoice && choice <= maxChoice)
        {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return choice;
        }

        cout << "Invalid menu choice. Try again.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

// -------------------- Input Helpers --------------------
string getNonEmptyLine(const string& prompt)
{
    string value;
    do
    {
        cout << prompt;
        getline(cin, value);

        if (value.empty())
            cout << "Input cannot be empty. Please try again.\n";

    } while (value.empty());

    return value;
}

int getValidatedInt(const string& prompt, int minVal, int maxVal)
{
    int value;
    while (true)
    {
        cout << prompt;

        if (cin >> value && value >= minVal && value <= maxVal)
        {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return value;
        }

        cout << "Invalid number. Enter " << minVal << " to " << maxVal << ".\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

double getValidatedDouble(const string& prompt, double minVal, double maxVal)
{
    double value;
    while (true)
    {
        cout << prompt;

        if (cin >> value && value >= minVal && value <= maxVal)
        {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return value;
        }

        cout << "Invalid number. Enter " << minVal << " to " << maxVal << ".\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

// -------------------- Enum Helpers --------------------
EnergyLevel getEnergyFromUser()
{
    cout << "Energy Level:\n";
    cout << "  1) Low\n";
    cout << "  2) Medium\n";
    cout << "  3) High\n";

    int choice = getValidatedInt("Choose energy (1-3): ", 1, 3);

    switch (choice)
    {
    case 1: return LOW;
    case 2: return MEDIUM;
    case 3: return HIGH;
    default: return MEDIUM;
    }
}

string energyToString(EnergyLevel e)
{
    if (e == LOW) return "Low";
    if (e == MEDIUM) return "Medium";
    return "High";
}

EnergyLevel energyFromString(const string& energyText)
{
    if (energyText == "Low" || energyText == "low")
        return LOW;
    if (energyText == "Medium" || energyText == "medium")
        return MEDIUM;
    if (energyText == "High" || energyText == "high")
        return HIGH;

    throw DJException("Invalid energy value in JSON: " + energyText);
}

// -------------------- Output Helpers (Weeks 1-4) --------------------
void printLegacyTableHeader(ostream& out)
{
    out << left
        << setw(TITLE_W) << "Title"
        << setw(ARTIST_W) << "Artist"
        << setw(GENRE_W) << "Genre"
        << setw(KEY_W) << "Key"
        << right << setw(6) << "BPM" << "  "
        << left << setw(8) << "Energy"
        << setw(NOTE_W) << "Notes"
        << "\n";

    out << string(LINE_W, '-') << "\n";
}

void printTrackRow(ostream& out, const Track& t)
{
    out << left
        << setw(TITLE_W) << t.title.substr(0, TITLE_W - 1)
        << setw(ARTIST_W) << t.artist.substr(0, ARTIST_W - 1)
        << setw(GENRE_W) << t.genre.substr(0, GENRE_W - 1)
        << setw(KEY_W) << t.key.substr(0, KEY_W - 1)
        << right << setw(6) << t.bpm << "  "
        << left << setw(8) << energyToString(t.energy)
        << setw(NOTE_W) << t.notes.substr(0, NOTE_W - 1)
        << "\n";
}

// -------------------- Main Features (Weeks 1-4) --------------------
void addTrack(Track library[], int& count)
{
    if (count >= MAX_TRACKS)
    {
        cout << "Library is full (" << MAX_TRACKS << " tracks). Cannot add more.\n";
        return;
    }

    cout << "\n--- Add Track (" << (count + 1) << "/" << MAX_TRACKS << ") ---\n";

    Track t;
    t.title = getNonEmptyLine("Title: ");
    t.artist = getNonEmptyLine("Artist: ");
    t.genre = getNonEmptyLine("Genre: ");
    t.key = getNonEmptyLine("Key (ex: Am, C, F#m): ");
    t.bpm = getValidatedInt("BPM (60-200): ", BPM_MIN, BPM_MAX);
    t.energy = getEnergyFromUser();
    t.notes = getNonEmptyLine("Notes (mix notes): ");

    library[count] = t;
    count++;

    cout << "Track added!\n";
}

void printLibrary(const Track library[], int count)
{
    if (count == 0)
    {
        cout << "No tracks saved yet.\n";
        return;
    }

    cout << "\n==================== LIBRARY (Weeks 1-4) ====================\n";
    printLegacyTableHeader(cout);

    for (int i = 0; i < count; i++)
        printTrackRow(cout, library[i]);

    double avg = computeAverageBPM(library, count);
    cout << "\nAverage BPM: " << fixed << setprecision(1) << avg << "\n";

    string checkGenre = getNonEmptyLine("Enter a genre to count matches: ");
    int matches = countGenreMatches(library, count, checkGenre);
    cout << "Tracks in genre \"" << checkGenre << "\": " << matches << "\n";
}

void recommendNextTracks(const Track library[], int count)
{
    if (count == 0)
    {
        cout << "No tracks in library. Add tracks first.\n";
        return;
    }

    cout << "\n--- Recommend Next Tracks ---\n";
    int currentBPM = getValidatedInt("Current BPM you are playing (60-200): ", BPM_MIN, BPM_MAX);
    EnergyLevel currentEnergy = getEnergyFromUser();

    const int BPM_RANGE = 5;
    bool found = false;

    cout << "\nSuggested tracks (within +/-" << BPM_RANGE
        << " BPM and energy stays steady or rises):\n";

    for (int i = 0; i < count; i++)
    {
        int bpmDiff = absValue(library[i].bpm - currentBPM);

        if (bpmDiff <= BPM_RANGE &&
            (library[i].energy == currentEnergy || library[i].energy == currentEnergy + 1))
        {
            found = true;
            cout << " - " << library[i].title << " by " << library[i].artist
                << " (" << library[i].bpm << " BPM, " << energyToString(library[i].energy) << ")\n";
        }
    }

    if (!found)
        cout << "No close matches found. Try adding more tracks.\n";
}

void saveReportToFile(const Track library[], int count, const string& filename)
{
    ofstream fout(filename.c_str());
    if (!fout)
    {
        cout << "Could not open file: " << filename << "\n";
        return;
    }

    fout << "==================== DJ SET ARCHITECT REPORT (Weeks 1-4) ====================\n";
    fout << "Tracks stored: " << count << "\n\n";

    if (count == 0)
    {
        fout << "No tracks saved.\n";
        fout.close();
        cout << "Report saved to " << filename << "\n";
        return;
    }

    printLegacyTableHeader(fout);
    for (int i = 0; i < count; i++)
        printTrackRow(fout, library[i]);

    double avg = computeAverageBPM(library, count);
    fout << "\nAverage BPM: " << fixed << setprecision(1) << avg << "\n";

    fout.close();
    cout << "Report saved to " << filename << "\n";
}

// -------------------- Calculations / Derived Values (Weeks 1-4) --------------------
double computeAverageBPM(const Track library[], int count)
{
    if (count == 0) return 0.0;

    int sum = 0;
    for (int i = 0; i < count; i++)
        sum += library[i].bpm;

    return static_cast<double>(sum) / static_cast<double>(count);
}

int countGenreMatches(const Track library[], int count, const string& genre)
{
    int matches = 0;
    for (int i = 0; i < count; i++)
    {
        if (library[i].genre == genre)
            matches++;
    }
    return matches;
}

// -------------------- Week 5/6/7 Helper Output --------------------
void printWeek5TableHeader(ostream& out)
{
    out << left
        << "Idx "
        << setw(TITLE_W) << "Title"
        << setw(TYPE_W) << "Type"
        << right << setw(6) << "BPM" << "  "
        << left << setw(8) << "Energy"
        << setw(NOTE_W) << "Notes"
        << "  Source\n";

    printSeparator(out);
}

void printSeparator(ostream& out)
{
    out << string(LINE_W, '-') << "\n";
}

int safeIndexFromUser(const string& prompt, int size)
{
    if (size <= 0) return -1;
    return getValidatedInt(prompt, 0, size - 1);
}

// -------------------- Doctest Unit Tests --------------------
#ifdef _DEBUG

Track makeTrack(const string& title, const string& genre, int bpm, EnergyLevel e)
{
    Track t;
    t.title = title;
    t.artist = "Test";
    t.genre = genre;
    t.key = "Am";
    t.bpm = bpm;
    t.energy = e;
    t.notes = "";
    return t;
}

void writeTextFile(const string& filename, const string& contents)
{
    ofstream fout(filename.c_str());
    fout << contents;
}

// -------------------- Existing Week 05 tests (keep) --------------------
TEST_CASE("Average BPM: 0 tracks returns 0.0")
{
    Track lib[1];
    CHECK(computeAverageBPM(lib, 0) == doctest::Approx(0.0));
}

TEST_CASE("Average BPM: 1 track")
{
    Track lib[1];
    lib[0] = makeTrack("A", "House", 120, MEDIUM);
    CHECK(computeAverageBPM(lib, 1) == doctest::Approx(120.0));
}

TEST_CASE("Average BPM: 2 tracks")
{
    Track lib[2];
    lib[0] = makeTrack("A", "House", 120, MEDIUM);
    lib[1] = makeTrack("B", "House", 140, HIGH);
    CHECK(computeAverageBPM(lib, 2) == doctest::Approx(130.0));
}

TEST_CASE("Average BPM: 3 tracks")
{
    Track lib[3];
    lib[0] = makeTrack("A", "House", 100, LOW);
    lib[1] = makeTrack("B", "Techno", 110, MEDIUM);
    lib[2] = makeTrack("C", "Techno", 130, HIGH);
    CHECK(computeAverageBPM(lib, 3) == doctest::Approx(113.3333333));
}

TEST_CASE("Energy enum prints correct strings")
{
    CHECK(energyToString(LOW) == "Low");
    CHECK(energyToString(MEDIUM) == "Medium");
    CHECK(energyToString(HIGH) == "High");
}

TEST_CASE("Genre matches: 0 matches")
{
    Track lib[2];
    lib[0] = makeTrack("A", "House", 120, MEDIUM);
    lib[1] = makeTrack("B", "Techno", 130, HIGH);

    CHECK(countGenreMatches(lib, 2, "Trance") == 0);
}

TEST_CASE("Genre matches: some matches")
{
    Track lib[3];
    lib[0] = makeTrack("A", "House", 120, MEDIUM);
    lib[1] = makeTrack("B", "House", 125, HIGH);
    lib[2] = makeTrack("C", "Techno", 130, HIGH);

    CHECK(countGenreMatches(lib, 3, "House") == 2);
}

TEST_CASE("Genre matches: all matches")
{
    Track lib[3];
    lib[0] = makeTrack("A", "House", 120, MEDIUM);
    lib[1] = makeTrack("B", "House", 125, HIGH);
    lib[2] = makeTrack("C", "House", 130, LOW);

    CHECK(countGenreMatches(lib, 3, "House") == 3);
}

TEST_CASE("TrackBase is abstract: test base behavior via derived")
{
    LocalTrack t("Test", 128, HIGH, "x.wav", MixNotes("n"));
    CHECK(t.getTitle() == "Test");
    CHECK(t.getBpm() == 128);
    CHECK(t.getEnergy() == HIGH);
    CHECK(t.getType() == "LocalTrack");
}

TEST_CASE("Polymorphism: base pointer calls derived override (StreamTrack)")
{
    TrackBase* p = new StreamTrack("S", 140, HIGH, "Apple Music", MixNotes("hi"));
    CHECK(p->getType() == "StreamTrack");

    ostringstream oss;
    p->print(oss);
    string out = oss.str();

    CHECK(out.find("StreamTrack") != string::npos);
    CHECK(out.find("Apple Music") != string::npos);

    delete p;
}

// -------------------- Week 06 tests (still valid) --------------------
TEST_CASE("operator==: equal LocalTrack objects")
{
    LocalTrack a("Song", 128, HIGH, "song.wav", MixNotes("x"));
    LocalTrack b("Song", 100, LOW, "song.wav", MixNotes("different notes"));
    CHECK(a == b);
}

TEST_CASE("operator==: not equal LocalTrack objects")
{
    LocalTrack a("Song", 128, HIGH, "song.wav", MixNotes("x"));
    LocalTrack b("Song", 128, HIGH, "other.wav", MixNotes("x"));
    CHECK((a == b) == false);
}

TEST_CASE("operator<<: outputs derived LocalTrack one-line summary")
{
    LocalTrack t("LocalName", 124, MEDIUM, "track.wav", MixNotes("n"));
    ostringstream oss;
    oss << t;
    string s = oss.str();
    CHECK(s.find("LocalTrack") != string::npos);
    CHECK(s.find("LocalName") != string::npos);
    CHECK(s.find("Path=track.wav") != string::npos);
}

TEST_CASE("operator<<: outputs derived StreamTrack one-line summary")
{
    StreamTrack t("StreamName", 140, HIGH, "Spotify", MixNotes("n"));
    ostringstream oss;
    oss << t;
    string s = oss.str();
    CHECK(s.find("StreamTrack") != string::npos);
    CHECK(s.find("StreamName") != string::npos);
    CHECK(s.find("Platform=Spotify") != string::npos);
}

// -------------------- Week 07 doctest updates (THROWS) --------------------

// TrackManager operator[]: valid index returns correct pointer
TEST_CASE("Week07 operator[]: valid index returns correct item pointer")
{
    TrackManager m(2);
    m += new LocalTrack("A", 120, MEDIUM, "a.wav", MixNotes(""));

    TrackBase* p = m[0];
    CHECK(p != nullptr);
    CHECK(p->getType() == "LocalTrack");
    CHECK(p->getTitle() == "A");
}

// TrackManager operator[]: invalid index throws
TEST_CASE("Week07 operator[]: invalid index throws")
{
    TrackManager m(2);
    m += new LocalTrack("A", 120, MEDIUM, "a.wav", MixNotes(""));

    CHECK_THROWS(m[99]);
    CHECK_THROWS(m[-1]);
}

// operator+= add increases size / stores correct pointer
TEST_CASE("operator+= add increases size and stores correct pointer")
{
    TrackManager m(2);
    CHECK(m.getSize() == 0);

    m += new StreamTrack("B", 125, HIGH, "Spotify", MixNotes(""));
    CHECK(m.getSize() == 1);
    CHECK(m[0] != nullptr);
    CHECK(m[0]->getType() == "StreamTrack");
}

// operator-= valid removal deletes and shifts
TEST_CASE("Week07 operator-= valid removal shifts properly")
{
    TrackManager m(2);
    m += new LocalTrack("A", 120, MEDIUM, "a.wav", MixNotes(""));
    m += new StreamTrack("B", 125, HIGH, "Spotify", MixNotes(""));
    m += new LocalTrack("C", 130, HIGH, "c.wav", MixNotes(""));

    CHECK(m.getSize() == 3);

    m -= 1; // remove middle; should shift "C" to index 1
    CHECK(m.getSize() == 2);

    CHECK(m[0]->getTitle() == "A");
    CHECK(m[1]->getTitle() == "C");
}

// operator-= invalid removal throws
TEST_CASE("Week07 operator-= invalid removal throws")
{
    TrackManager m(2);
    m += new LocalTrack("A", 120, MEDIUM, "a.wav", MixNotes(""));
    CHECK_THROWS(m -= 5);
    CHECK_THROWS(m -= -1);
}

// Function template tests
TEST_CASE("Function template absValue works for int")
{
    CHECK(absValue(-10) == 10);
    CHECK(absValue(5) == 5);
}

TEST_CASE("Function template absValue works for double")
{
    CHECK(absValue(-2.5) == doctest::Approx(2.5));
    CHECK(absValue(3.25) == doctest::Approx(3.25));
}

// DynamicArray now throws on invalid access/removal (Week07)
TEST_CASE("Week07 DynamicArray<int>: store/remove/resizing still works")
{
    DynamicArray<int> a(2);
    CHECK(a.getSize() == 0);
    CHECK(a.getCapacity() == 2);

    a.pushBack(10);
    a.pushBack(20);
    a.pushBack(30); // resize
    CHECK(a.getSize() == 3);
    CHECK(a.getCapacity() >= 3);

    CHECK(a.at(0) == 10);
    CHECK(a.at(1) == 20);
    CHECK(a.at(2) == 30);

    a.removeAt(1);
    CHECK(a.getSize() == 2);
    CHECK(a.at(0) == 10);
    CHECK(a.at(1) == 30);
}

TEST_CASE("Week07 DynamicArray throws on invalid access/removal")
{
    DynamicArray<int> a(2);
    a.pushBack(1);

    CHECK_THROWS(a.at(5));
    CHECK_THROWS(a.at(-1));
    CHECK_THROWS(a.removeAt(2));
    CHECK_THROWS(a.removeAt(-1));
}

TEST_CASE("Week07 DynamicArray<TrackBase*> basic works and throws on invalid")
{
    DynamicArray<TrackBase*> a(2);
    TrackBase* p1 = new LocalTrack("A", 120, MEDIUM, "a.wav", MixNotes(""));
    TrackBase* p2 = new StreamTrack("B", 125, HIGH, "Spotify", MixNotes(""));

    a.pushBack(p1);
    a.pushBack(p2);

    CHECK(a.getSize() == 2);
    CHECK(a.at(0)->getType() == "LocalTrack");
    CHECK(a.at(1)->getType() == "StreamTrack");
    CHECK_THROWS(a.at(99));
    CHECK_THROWS(a.removeAt(99));

    // Clean up (template does not own pointed-to objects)
    delete p1;
    delete p2;
}

// Week 08 tests

TEST_CASE("Week08 recursive countHighEnergyRecursive: counts HIGH tracks correctly")
{
    TrackManager m(2);
    m += new LocalTrack("A", 120, LOW, "a.wav", MixNotes(""));
    m += new StreamTrack("B", 125, HIGH, "Spotify", MixNotes(""));
    m += new LocalTrack("C", 130, HIGH, "c.wav", MixNotes(""));
    m += new StreamTrack("D", 128, MEDIUM, "Apple Music", MixNotes(""));

    CHECK(m.countHighEnergyRecursive() == 2);
}

TEST_CASE("Week08 recursive countHighEnergyRecursive: no HIGH tracks returns 0")
{
    TrackManager m(2);
    m += new LocalTrack("A", 120, LOW, "a.wav", MixNotes(""));
    m += new StreamTrack("B", 125, MEDIUM, "Spotify", MixNotes(""));

    CHECK(m.countHighEnergyRecursive() == 0);
}

TEST_CASE("Week08 recursive countHighEnergyRecursive: empty manager returns 0")
{
    TrackManager m(2);
    CHECK(m.countHighEnergyRecursive() == 0);
}

// ==================== Week 09 Tests: vector, sequential search, sort, binary search ====================

// Test 1: vector .push_back() grows bpmList; .at() retrieves correct values
TEST_CASE("Week09 vector: push_back stores BPMs and .at() retrieves them")
{
    TrackManager m(2);
    CHECK(m.getBpmCount() == 0); // empty vector (edge case)

    m += new LocalTrack("A", 120, MEDIUM, "a.wav", MixNotes(""));
    m += new StreamTrack("B", 135, HIGH, "Spotify", MixNotes(""));

    CHECK(m.getBpmCount() == 2);    // .size() via getBpmCount()
    CHECK(m.getBpmAt(0) == 120);    // .at(0)
    CHECK(m.getBpmAt(1) == 135);    // .at(1)
}

// Test 2: sequential search — found returns index; not found or empty returns -1
TEST_CASE("Week09 sequentialSearchBpm: found returns index, not found or empty returns -1")
{
    TrackManager empty(2);
    CHECK(empty.sequentialSearchBpm(128) == -1); // edge case: empty vector

    TrackManager m(2);
    m += new LocalTrack("A", 120, MEDIUM, "a.wav", MixNotes(""));
    m += new StreamTrack("B", 135, HIGH, "Spotify", MixNotes(""));

    CHECK(m.sequentialSearchBpm(135) == 1);  // found
    CHECK(m.sequentialSearchBpm(999) == -1); // not found
}

// Test 3: bubble sort — out-of-order BPMs sorted ascending
TEST_CASE("Week09 sortBpmsBubble: sorts BPM values into ascending order")
{
    TrackManager m(2);
    m += new LocalTrack("A", 150, HIGH, "a.wav", MixNotes(""));
    m += new StreamTrack("B", 120, MEDIUM, "Spotify", MixNotes(""));
    m += new LocalTrack("C", 135, HIGH, "c.wav", MixNotes(""));

    m.sortBpmsBubble();

    CHECK(m.getBpmAt(0) == 120);
    CHECK(m.getBpmAt(1) == 135);
    CHECK(m.getBpmAt(2) == 150);
}

// Test 4: binary search — requires sort first; finds element or returns -1
TEST_CASE("Week09 binarySearchBpm: finds element after sort; returns -1 if not found or empty")
{
    TrackManager empty(2);
    CHECK(empty.binarySearchBpm(128) == -1); // edge case: empty vector

    TrackManager m(2);
    m += new LocalTrack("A", 150, HIGH, "a.wav", MixNotes(""));
    m += new StreamTrack("B", 120, MEDIUM, "Spotify", MixNotes(""));
    m += new LocalTrack("C", 135, HIGH, "c.wav", MixNotes(""));

    m.sortBpmsBubble(); // must sort before binary search — [120, 135, 150]
    CHECK(m.binarySearchBpm(135) == 1);  // found at index 1
    CHECK(m.binarySearchBpm(999) == -1); // not found
}

// ==================== Week 10 Tests: custom linked list + iterator ====================

TEST_CASE("Week10 linked list insert: insert into empty list and use two positions")
{
    BpmLinkedList list;

    // Empty -> first insert at front
    list.insertFront(120);
    CHECK(list.getSize() == 1);
    CHECK(list.at(0) == 120);

    // Second insert at back (different insertion position)
    list.insertBack(130);
    CHECK(list.getSize() == 2);
    CHECK(list.at(0) == 120);
    CHECK(list.at(1) == 130);
}

TEST_CASE("Week10 linked list delete: deleting non-existent value returns false")
{
    BpmLinkedList list;
    list.insertBack(120);
    list.insertBack(130);

    CHECK(list.removeFirstMatch(999) == false); // edge case: node does not exist
    CHECK(list.getSize() == 2);                 // list remains unchanged
}

TEST_CASE("Week10 linked list search + iterator traversal order")
{
    BpmLinkedList list;
    list.insertBack(100);
    list.insertBack(110);
    list.insertBack(120);

    CHECK(list.search(100) == 0);
    CHECK(list.search(120) == 2);
    CHECK(list.search(999) == -1);

    // Iterator requirement: front initialization, next(), current data access.
    BpmListIterator it = list.begin();
    CHECK(it.isValid());
    CHECK(it.data() == 100);
    it.next();
    CHECK(it.data() == 110);
    it.next();
    CHECK(it.data() == 120);
}

TEST_CASE("Week10 linked list traverse/print empty list")
{
    BpmLinkedList list;
    ostringstream oss;
    list.print(oss); // edge case: traverse empty linked list
    CHECK(oss.str() == "(empty)\n");

    BpmListIterator it = list.begin();
    CHECK(it.isValid() == false);
}

// ==================== Week 11 Tests: stack + queue ====================

TEST_CASE("Week11 ArrayStack push top pop maintains LIFO order")
{
    ArrayStack<int> stack(3);
    CHECK(stack.isEmpty());

    stack.push(10);
    stack.push(20);
    stack.push(30);

    CHECK(stack.getSize() == 3);
    CHECK(stack.top() == 30);

    stack.pop();
    CHECK(stack.top() == 20);

    stack.pop();
    CHECK(stack.top() == 10);

    stack.pop();
    CHECK(stack.isEmpty());
}

TEST_CASE("Week11 ArrayStack throws on full push and empty pop or top")
{
    ArrayStack<int> stack(2);
    stack.push(1);
    stack.push(2);

    CHECK_THROWS(stack.push(3));

    stack.pop();
    stack.pop();

    CHECK_THROWS(stack.pop());
    CHECK_THROWS(stack.top());
}

TEST_CASE("Week11 ArrayQueue enqueue front dequeue maintains FIFO order")
{
    ArrayQueue<string> queue(3);
    CHECK(queue.isEmpty());

    queue.enqueue("A");
    queue.enqueue("B");
    queue.enqueue("C");

    CHECK(queue.getSize() == 3);
    CHECK(queue.front() == "A");

    queue.dequeue();
    CHECK(queue.front() == "B");

    queue.dequeue();
    CHECK(queue.front() == "C");

    queue.dequeue();
    CHECK(queue.isEmpty());
}

TEST_CASE("Week11 ArrayQueue throws on full enqueue and empty dequeue or front")
{
    ArrayQueue<int> queue(2);
    queue.enqueue(100);
    queue.enqueue(200);

    CHECK_THROWS(queue.enqueue(300));

    queue.dequeue();
    queue.dequeue();

    CHECK_THROWS(queue.dequeue());
    CHECK_THROWS(queue.front());
}

TEST_CASE("Week11 ArrayQueue handles circular wraparound correctly")
{
    ArrayQueue<int> queue(3);
    queue.enqueue(1);
    queue.enqueue(2);
    queue.enqueue(3);

    queue.dequeue();
    queue.dequeue();

    queue.enqueue(4);
    queue.enqueue(5);

    CHECK(queue.front() == 3);
    queue.dequeue();
    CHECK(queue.front() == 4);
    queue.dequeue();
    CHECK(queue.front() == 5);
}

TEST_CASE("Week11 TrackManager prints stack and queue views using existing tracks")
{
    TrackManager m(2);
    m += new LocalTrack("Alpha", 120, MEDIUM, "a.wav", MixNotes(""));
    m += new StreamTrack("Bravo", 125, HIGH, "Spotify", MixNotes(""));
    m += new LocalTrack("Charlie", 130, HIGH, "c.wav", MixNotes(""));

    ostringstream stackOut;
    m.printRecentAdditionsStack(stackOut);
    CHECK(stackOut.str() == "Recent additions stack (LIFO): Charlie -> Bravo -> Alpha\n");

    ostringstream queueOut;
    m.printPlaybackQueue(queueOut);
    CHECK(queueOut.str() == "Playback queue (FIFO): Alpha -> Bravo -> Charlie\n");
}

TEST_CASE("Week11 TrackManager prints empty stack and queue views")
{
    TrackManager m(2);

    ostringstream stackOut;
    m.printRecentAdditionsStack(stackOut);
    CHECK(stackOut.str() == "Recent additions stack (LIFO): (empty)\n");

    ostringstream queueOut;
    m.printPlaybackQueue(queueOut);
    CHECK(queueOut.str() == "Playback queue (FIFO): (empty)\n");
}

// ==================== Week 12 Tests: STL map title index ====================

TEST_CASE("Week12 title map insert and lookup finds existing title")
{
    TrackManager m(2);
    CHECK(m.getTitleMapCount() == 0);

    m += new LocalTrack("Alpha", 120, MEDIUM, "a.wav", MixNotes(""));
    m += new StreamTrack("Bravo", 125, HIGH, "Spotify", MixNotes(""));

    CHECK(m.getTitleMapCount() == 2);

    const TrackBase* found = m.findTrackByTitle("Bravo");
    REQUIRE(found != nullptr);
    CHECK(found->getTitle() == "Bravo");
    CHECK(found->getBpm() == 125);
}

TEST_CASE("Week12 title map lookup missing title returns nullptr")
{
    TrackManager m(2);
    m += new LocalTrack("Alpha", 120, MEDIUM, "a.wav", MixNotes(""));

    CHECK(m.findTrackByTitle("Missing") == nullptr);
}

TEST_CASE("Week12 title map delete existing and missing title")
{
    TrackManager m(2);
    m += new LocalTrack("Alpha", 120, MEDIUM, "a.wav", MixNotes(""));
    m += new StreamTrack("Bravo", 125, HIGH, "Spotify", MixNotes(""));

    CHECK(m.removeTrackByTitle("Missing") == false); // edge case: key does not exist
    CHECK(m.getSize() == 2);
    CHECK(m.getTitleMapCount() == 2);

    CHECK(m.removeTrackByTitle("Alpha") == true);
    CHECK(m.getSize() == 1);
    CHECK(m.getTitleMapCount() == 1);
    CHECK(m.findTrackByTitle("Alpha") == nullptr);
    CHECK(m[0]->getTitle() == "Bravo");
}

TEST_CASE("Week12 title map iteration prints sorted key value pairs")
{
    TrackManager m(2);
    m += new StreamTrack("Bravo", 125, HIGH, "Spotify", MixNotes(""));
    m += new LocalTrack("Alpha", 120, MEDIUM, "a.wav", MixNotes(""));

    ostringstream oss;
    m.printTitleMap(oss);
    CHECK(oss.str() == "Title map lookup (std::map): Alpha => index 1 | Bravo => index 0\n");

    TrackManager empty(2);
    ostringstream emptyOut;
    empty.printTitleMap(emptyOut);
    CHECK(emptyOut.str() == "Title map lookup (std::map): (empty)\n");
}

TEST_CASE("Week13 JSON load reads disk file into existing manager structures")
{
    const string filename = "week13_valid_tracks.json";
    writeTextFile(
        filename,
        "["
        "{\"type\":\"local\",\"title\":\"Sunrise Intro\",\"bpm\":120,\"energy\":\"Low\",\"source\":\"sunrise.wav\",\"notes\":\"Warm opener\"},"
        "{\"type\":\"stream\",\"title\":\"Neon Drive\",\"bpm\":124,\"energy\":\"Medium\",\"source\":\"Spotify\",\"notes\":\"Build momentum\"},"
        "{\"type\":\"local\",\"title\":\"Warehouse Lift\",\"bpm\":128,\"energy\":\"High\",\"source\":\"warehouse.wav\",\"notes\":\"Peak groove\"},"
        "{\"type\":\"stream\",\"title\":\"Pulse Check\",\"bpm\":126,\"energy\":\"Medium\",\"source\":\"Apple Music\",\"notes\":\"Reset tension\"},"
        "{\"type\":\"local\",\"title\":\"Closing Lights\",\"bpm\":122,\"energy\":\"Low\",\"source\":\"closing.wav\",\"notes\":\"Smooth finish\"}"
        "]"
    );

    TrackManager m(2);
    string status;
    const bool loaded = m.loadTracksFromJsonFile(filename, status);

    CHECK(loaded == true);
    CHECK(status.find("Loaded 5 tracks") != string::npos);
    CHECK(m.getSize() == 5);
    CHECK(m.getTitleMapCount() == 5);
    CHECK(m.getBpmCount() == 5);
    CHECK(m.sequentialSearchBpm(128) == 2);

    const TrackBase* found = m.findTrackByTitle("Neon Drive");
    REQUIRE(found != nullptr);
    CHECK(found->getType() == "StreamTrack");
    CHECK(found->getBpm() == 124);

    remove(filename.c_str());
}

TEST_CASE("Week13 JSON load handles missing file with try catch path")
{
    TrackManager m(2);
    string status;

    const bool loaded = m.loadTracksFromJsonFile("missing_week13_tracks.json", status);

    CHECK(loaded == false);
    CHECK(status.find("Could not open JSON file") != string::npos);
    CHECK(m.getSize() == 0);
}

TEST_CASE("Week13 JSON load handles malformed JSON with try catch path")
{
    const string filename = "week13_bad_tracks.json";
    writeTextFile(filename, "[{\"type\":\"local\",\"title\":\"Broken\"");

    TrackManager m(2);
    string status;
    const bool loaded = m.loadTracksFromJsonFile(filename, status);

    CHECK(loaded == false);
    CHECK(status.empty() == false);
    CHECK(m.getSize() == 0);

    remove(filename.c_str());
}

#endif
