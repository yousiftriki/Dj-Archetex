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
  * TrackManager operator[] with bounds checking (no exceptions)
  * TrackManager operator+= / operator-= to add/remove items
  * Explicit use of this pointer
- Templates:
  * Function template used in the program
  * Class template replaces Week 05 dynamic array logic
- Doctests updated for:
  * == tests, << tests, [] tests, +=/-= tests, template function tests, class template tests


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
*/

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

using namespace std;

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

// Menu range (merged menu: original + week5)
const int MENU_MIN = 1;
const int MENU_MAX = 10;

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

// -------------------- Weeks 1-4 Features --------------------
void addTrack(Track library[], int& count);
void printLibrary(const Track library[], int count);
void recommendNextTracks(const Track library[], int count);
void saveReportToFile(const Track library[], int count, const string& filename);

// Derived values / calculations
double computeAverageBPM(const Track library[], int count);
int countGenreMatches(const Track library[], int count, const string& genre);

// Output helpers (Weeks 1-4)
void printLegacyTableHeader(ostream& out);
void printTrackRow(ostream& out, const Track& t);

// -------------------- Week 5/6 Helpers --------------------
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

    // helper method (required)
    bool hasNotes() const
    {
        return !notes.empty();
    }
};


// -------------------- Week 06: Class Template (replaces Week 05 dynamic array logic) --------------------
// REQUIREMENT: Create a class template that replaces the Week 05 dynamic array logic.
//
// This template stores items of ANY type T using a dynamic array:
// - tracks size + capacity
// - resizes as needed
// - supports pushBack and removeAt (shift close gap)
//
// NOTE: This template does NOT delete pointed-to objects.
// Ownership and deletion logic belongs to TrackManager (because it "owns" the TrackBase* objects).
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

        // Copy old elements
        for (int i = 0; i < size; i++)
            newItems[i] = items[i];

        delete[] items;
        items = newItems;
        capacity = newCap;
    }

    // Prevent copying to avoid double-free of internal array
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

    // Removes an item by index and shifts everything left.
    // NOTE: This does NOT delete object memory if T is a pointer type.
    bool removeAt(int index)
    {
        if (index < 0 || index >= size)
            return false;

        for (int i = index; i < size - 1; i++)
            items[i] = items[i + 1];

        size--;

        // Optional shrink: keep it simple and safe
        if (size > 0 && size <= capacity / 4 && capacity > 2)
            resize(capacity / 2);

        return true;
    }

    // Safe access: returns default T() if invalid (no exceptions; matches course constraints).
    T at(int index) const
    {
        if (index < 0 || index >= size)
            return T(); // for pointers => nullptr, for int => 0, etc.
        return items[index];
    }

    // Provide direct access for internal use (still be careful).
    // (We keep it minimal; manager does bounds checks for public operator[].)
    T& rawAt(int index) { return items[index]; }
    const T& rawAt(int index) const { return items[index]; }

    ~DynamicArray()
    {
        delete[] items;
    }
};

// -------------------- Week 5/6: Abstract Base Class --------------------
// TrackBase is now an ABSTRACT base class.
class TrackBase
{
protected:
    string title;  // required protected for derived access

private:
    int bpm;
    EnergyLevel energy;

public:
    TrackBase() : title(""), bpm(0), energy(MEDIUM) {}

    TrackBase(const string& t, int b, EnergyLevel e)
        : title(t), bpm(b), energy(e) {
    }

    // getters/setters
    string getTitle() const { return title; }
    int getBpm() const { return bpm; }
    EnergyLevel getEnergy() const { return energy; }

    void setTitle(const string& t) { title = t; }
    void setBpm(int b) { bpm = b; }
    void setEnergy(EnergyLevel e) { energy = e; }

    // Week 5: print stays, but must be virtual (polymorphism)
    virtual void print(ostream& out) const
    {
        out << left
            << setw(TITLE_W) << title.substr(0, TITLE_W - 1)
            << setw(TYPE_W) << getType()
            << right << setw(6) << bpm << "  "
            << left << setw(8) << energyToString(energy);
    }

    // Week 6: virtual function used by operator<< (polymorphic one-line summary)
   // REQUIREMENT: << should call a virtual function so correct derived behavior runs.
    virtual void toStream(ostream& out) const
    {
        // One-line summary base portion (common fields)
        out << getType() << " | " << title << " | " << bpm << " BPM | " << energyToString(energy);
    }


    // Week 5: PURE VIRTUAL FUNCTION (makes class abstract)
    virtual string getType() const = 0;

    // Week 5: virtual destructor (critical for deleting via base pointer)
    virtual ~TrackBase() {}
};

// -------------------- Week 06: operator<< overload (polymorphic) --------------------
// REQUIREMENT: Overload operator<< for base or derived.
// It must output a clean one-line summary.
// Must use polymorphism: << calls virtual toStream().
ostream& operator<<(ostream& out, const TrackBase& t)
{
    t.toStream(out);  // virtual call => derived override runs
    return out;
}

// -------------------- Week 5/6: Derived Class #1 --------------------
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
        out << setw(KEY_W) << "" // spacer
            << setw(NOTE_W) << (notes.hasNotes() ? notes.getNotes().substr(0, NOTE_W - 1) : "(none)")
            << "  Path: " << filePath;
    }

    // Week 06: override toStream for polymorphic operator<< output
    void toStream(ostream& out) const override
    {
        // Build from base meaning + add derived details
        out << getType()
            << " | " << title
            << " | " << getBpm() << " BPM"
            << " | " << energyToString(getEnergy())
            << " | Path=" << filePath;
    }

    // Week 06: operator== for at least one derived class
    // REQUIREMENT: "Two objects compare equal when meaningful identity fields match."
    //
    // Identity choice for LocalTrack:
    // - title and filePath uniquely identify a local file track in this program.
    bool operator==(const LocalTrack& other) const
    {
        return (this->title == other.title) && (this->filePath == other.filePath);
    }
};

// -------------------- Week 5/6: Derived Class #2 --------------------
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
        out << setw(KEY_W) << "" // spacer
            << setw(NOTE_W) << (notes.hasNotes() ? notes.getNotes().substr(0, NOTE_W - 1) : "(none)")
            << "  Platform: " << platform;
    }

    // Week 06: override toStream for polymorphic operator<< output
    void toStream(ostream& out) const override
    {
        out << getType()
            << " | " << title
            << " | " << getBpm() << " BPM"
            << " | " << energyToString(getEnergy())
            << " | Platform=" << platform;
    }
};

// -------------------- Week 5/6: Manager Class --------------------
// TrackManager OWNS TrackBase* objects and uses a class template for storage.
// Week 06 change: Replace TrackBase** + manual resize with DynamicArray<TrackBase*>.
class TrackManager
{
private:
    DynamicArray<TrackBase*> items; // template-based dynamic container

    // Prevent copying to avoid double-free of owned TrackBase* objects
    TrackManager(const TrackManager&) = delete;
    TrackManager& operator=(const TrackManager&) = delete;

public:
    TrackManager(int cap = 2)
        : items(cap)
    {
    }

    int getSize() const { return items.getSize(); }
    int getCapacity() const { return items.getCapacity(); }

    // Adds a pointer (manager takes ownership)
    void add(TrackBase* p)
    {
        items.pushBack(p);
    }

    // Removes by index (deletes object, shifts close the gap)
    bool removeAt(int index)
    {
        if (index < 0 || index >= items.getSize())
            return false;

        // delete owned object
        TrackBase* doomed = items.at(index);
        delete doomed;

        // remove pointer slot and shift
        return items.removeAt(index);
    }

    // Week 06: operator[] with bounds checking (no exceptions)
    // REQUIREMENT: If invalid index, handle safely and do not throw.
    // Rule: return nullptr if invalid.
    TrackBase* operator[](int index) const
    {
        return items.at(index); // at() returns default (nullptr) if invalid
    }

    // Week 06: operator+= adds item pointer to container
    TrackManager& operator+=(TrackBase* p)
    {
        // REQUIREMENT: explicitly use this pointer at least once
        this->add(p);
        return *this;
    }

    // Week 06: operator-= removes item by index
    TrackManager& operator-=(int index)
    {
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
            TrackBase* p = items.at(i);
            if (p)
                p->print(out); // polymorphic print
            out << "\n";
        }

        printSeparator(out);
    }

    void saveReport(const string& filename) const
    {
        ofstream fout(filename.c_str());
        if (!fout)
        {
            cout << "Could not open file: " << filename << "\n";
            return;
        }

        fout << "==================== DJ SET ARCHITECT REPORT (Week 6) ====================\n";
        fout << "Tracks stored: " << items.getSize() << "\n\n";

        printAll(fout);

        fout.close();
        cout << "Report saved to " << filename << "\n";
    }

    ~TrackManager()
    {
        // delete all owned objects
        for (int i = 0; i < items.getSize(); i++)
        {
            TrackBase* p = items.at(i);
            delete p;
        }
        // DynamicArray destructor cleans up its internal array
    }
};

// -------------------- Main --------------------
#ifndef _DEBUG
int main()
{
    // Weeks 1-4 storage
    Track library[MAX_TRACKS];
    int trackCount = 0;

    // Week 5/6 storage
    TrackManager manager(2);

    showBanner();

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

    // do-while loop (rubric requirement): keep showing menu until user quits
    int choice = 0;
    do
    {
        showMenu();
        choice = getMenuChoice(MENU_MIN, MENU_MAX);

        // switch menu (rubric requirement)
        switch (choice)
        {
            // ---------------- Weeks 1-4 menu ----------------
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

            // ---------------- Week 5/6 menu ----------------
        case 5:
        {
            // Add LocalTrack (dynamic allocation required)
            cout << "\n--- Add Local Track (Week 6) ---\n";
            string t = getNonEmptyLine("Title: ");
            int bpm = getValidatedInt("BPM (60-200): ", BPM_MIN, BPM_MAX);
            EnergyLevel e = getEnergyFromUser();
            string path = getNonEmptyLine("File path (ex: track.wav): ");
            string noteText = getNonEmptyLine("Notes (mix notes): ");

            // Week 06 operator+= usage (adds and takes ownership)
            manager += new LocalTrack(t, bpm, e, path, MixNotes(noteText));
            cout << "Local track added (Week 6).\n";
            break;
        }
        case 6:
        {
            // Add StreamTrack (dynamic allocation required)
            cout << "\n--- Add Stream Track (Week 6) ---\n";
            string t = getNonEmptyLine("Title: ");
            int bpm = getValidatedInt("BPM (60-200): ", BPM_MIN, BPM_MAX);
            EnergyLevel e = getEnergyFromUser();
            string platform = getNonEmptyLine("Platform (ex: Spotify): ");
            string noteText = getNonEmptyLine("Notes (mix notes): ");

            // Week 06 operator+= usage
            manager += new StreamTrack(t, bpm, e, platform, MixNotes(noteText));
            cout << "Stream track added (Week 6).\n";
            break;
        }
        case 7:
            cout << "\n==================== WEEK 6 LIBRARY ====================\n";
            manager.printAll(cout);

            // Show operator<< output quickly (polymorphic one-line)
            if (manager.getSize() > 0 && manager[0] != nullptr)
                cout << "One-line summary (operator<<): " << *manager[0] << "\n";
            break;
        case 8:
        {
            cout << "\n--- Remove Week 6 Track ---\n";
            if (manager.getSize() == 0)
            {
                cout << "Nothing to remove.\n";
                break;
            }

            manager.printAll(cout);
            int idx = safeIndexFromUser("Enter index to remove: ", manager.getSize());

            // Week 06 operator-= usage (removes safely)
            manager -= idx;
            cout << "Remove attempted at index " << idx << ".\n";
            break;
        }
        case 9:
            manager.saveReport("DJ_Set_Report_Week6.txt");
            break;

            // Quit
        case 10:
            cout << "\nGoodbye, " << djName << "! Keep the crowd moving.\n";
            break;

        default:
            cout << "Invalid choice.\n";
        }

    } while (choice != 10);

    return 0;
}
#endif

// -------------------- UI Functions --------------------
void showBanner()
{
    cout << "=============================================\n";
    cout << "        DJ SET ARCHITECT - C++ EDITION       \n";
    cout << "   Weeks 1-4 + Week 5 OOP Upgrade Combined   \n";
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

    cout << "WEEK 5 (Abstract + Polymorphism)\n";
    cout << "5) Add Local Track (Week 5)\n";
    cout << "6) Add Stream Track (Week 5)\n";
    cout << "7) View Week 5 library (polymorphic print)\n";
    cout << "8) Remove Week 5 track by index\n";
    cout << "9) Save Week 5 report to file\n\n";

    cout << "10) Quit\n";
    cout << "----------------------------------------------\n";
}

// Validates menu input using a while loop and stream error handling.
int getMenuChoice(int minChoice, int maxChoice)
{
    int choice;
    while (true)
    {
        cout << "Enter choice (" << minChoice << "-" << maxChoice << "): ";

        // Accept only an integer in range
        if (cin >> choice && choice >= minChoice && choice <= maxChoice)
        {
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // clear newline
            return choice;
        }

        // Handle invalid input
        cout << "Invalid menu choice. Try again.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

// -------------------- Input Helpers --------------------

// Gets a non-empty full line of text (supports spaces).
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

// Gets a valid integer between minVal and maxVal.
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

// Gets a valid double between minVal and maxVal.
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

// Prompts user for energy choice and converts it into the enum.
// Uses a switch decision structure (rubric requirement).
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

// Converts an EnergyLevel into a printable string.
string energyToString(EnergyLevel e)
{
    if (e == LOW) return "Low";
    if (e == MEDIUM) return "Medium";
    return "High";
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

    library[count] = t; // store struct into array
    count++;            // update how many tracks are stored

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

    // Derived value: average BPM
    double avg = computeAverageBPM(library, count);
    cout << "\nAverage BPM: " << fixed << setprecision(1) << avg << "\n";

    // Derived value: genre count
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

    const int BPM_RANGE = 5; // recommend within +/- 5 BPM
    bool found = false;

    cout << "\nSuggested tracks (within +/-" << BPM_RANGE
        << " BPM and energy stays steady or rises):\n";

    for (int i = 0; i < count; i++)
    {
        // Week 06 template usage: absValue<int>
        int bpmDiff = absValue(library[i].bpm - currentBPM);

        // Compound boolean condition: bpm close AND energy same or +1 step
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

// -------------------- Week 5/6 Helper Output --------------------
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

// Quick helper to make tracks easy in tests (Weeks 1-4)
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

    delete p; // virtual destructor ensures correct cleanup
}

TEST_CASE("Manager: add increases size and resizes")
{
    TrackManager m(2);
    CHECK(m.getSize() == 0);
    CHECK(m.getCapacity() == 2);

    m.add(new LocalTrack("A", 120, MEDIUM, "a.wav", MixNotes("note")));
    m.add(new StreamTrack("B", 125, HIGH, "Spotify", MixNotes("")));
    CHECK(m.getSize() == 2);
    CHECK(m.getCapacity() == 2);

    // triggers resize
    m.add(new LocalTrack("C", 130, HIGH, "c.wav", MixNotes("x")));
    CHECK(m.getSize() == 3);
    CHECK(m.getCapacity() >= 3);
}

TEST_CASE("Manager: remove deletes and shifts")
{
    TrackManager m(2);
    m.add(new LocalTrack("A", 120, MEDIUM, "a.wav", MixNotes("note")));
    m.add(new StreamTrack("B", 125, HIGH, "Spotify", MixNotes("")));
    m.add(new LocalTrack("C", 130, HIGH, "c.wav", MixNotes("x")));

    CHECK(m.getSize() == 3);
    CHECK(m.removeAt(1) == true);
    CHECK(m.getSize() == 2);

    ostringstream oss;
    m.printAll(oss);
    string out = oss.str();
    CHECK(out.find("Idx") != string::npos);
    CHECK(out.find("LocalTrack") != string::npos);
}

// -------------------- Week 06 NEW doctests (required) --------------------

// 1) Equality operator == (at least 2 tests)
TEST_CASE("operator==: equal LocalTrack objects")
{
    LocalTrack a("Song", 128, HIGH, "song.wav", MixNotes("x"));
    LocalTrack b("Song", 100, LOW, "song.wav", MixNotes("different notes"));
    // identity = title + filePath, so these should be equal
    CHECK(a == b);
}

TEST_CASE("operator==: not equal LocalTrack objects")
{
    LocalTrack a("Song", 128, HIGH, "song.wav", MixNotes("x"));
    LocalTrack b("Song", 128, HIGH, "other.wav", MixNotes("x"));
    CHECK((a == b) == false);
}

// 2) operator<< output (at least 2 tests)
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

// 3) operator[] indexing (at least 2 tests)
TEST_CASE("operator[]: valid index returns correct item pointer")
{
    TrackManager m(2);
    m += new LocalTrack("A", 120, MEDIUM, "a.wav", MixNotes(""));

    TrackBase* p = m[0];
    CHECK(p != nullptr);
    CHECK(p->getType() == "LocalTrack");
    CHECK(p->getTitle() == "A");
}

TEST_CASE("operator[]: invalid index returns nullptr")
{
    TrackManager m(2);
    m += new LocalTrack("A", 120, MEDIUM, "a.wav", MixNotes(""));
    CHECK(m[99] == nullptr);
    CHECK(m[-1] == nullptr);
}

// 4) += / -= add/remove (at least 2 tests)
TEST_CASE("operator+= add increases size and stores correct pointer")
{
    TrackManager m(2);
    CHECK(m.getSize() == 0);

    m += new StreamTrack("B", 125, HIGH, "Spotify", MixNotes(""));
    CHECK(m.getSize() == 1);
    CHECK(m[0] != nullptr);
    CHECK(m[0]->getType() == "StreamTrack");
}

TEST_CASE("operator-= remove deletes and shifts properly")
{
    TrackManager m(2);
    m += new LocalTrack("A", 120, MEDIUM, "a.wav", MixNotes(""));
    m += new StreamTrack("B", 125, HIGH, "Spotify", MixNotes(""));
    m += new LocalTrack("C", 130, HIGH, "c.wav", MixNotes(""));

    CHECK(m.getSize() == 3);

    // remove middle (index 1), should shift "C" into index 1
    m -= 1;
    CHECK(m.getSize() == 2);

    CHECK(m[0] != nullptr);
    CHECK(m[0]->getTitle() == "A");

    CHECK(m[1] != nullptr);
    CHECK(m[1]->getTitle() == "C");
}

// 5) Template usage: at least 2 tests (function template)
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

// 6) Class template usage: at least 2 tests
TEST_CASE("Class template DynamicArray works for int (store/remove/resizing)")
{
    DynamicArray<int> a(2);
    CHECK(a.getSize() == 0);
    CHECK(a.getCapacity() == 2);

    a.pushBack(10);
    a.pushBack(20);
    CHECK(a.getSize() == 2);

    // triggers resize
    a.pushBack(30);
    CHECK(a.getSize() == 3);
    CHECK(a.getCapacity() >= 3);

    CHECK(a.at(0) == 10);
    CHECK(a.at(1) == 20);
    CHECK(a.at(2) == 30);

    CHECK(a.removeAt(1) == true);
    CHECK(a.getSize() == 2);
    CHECK(a.at(0) == 10);
    CHECK(a.at(1) == 30);
}

TEST_CASE("Class template DynamicArray works for TrackBase* (store and safe at)")
{
    DynamicArray<TrackBase*> a(2);
    TrackBase* p1 = new LocalTrack("A", 120, MEDIUM, "a.wav", MixNotes(""));
    TrackBase* p2 = new StreamTrack("B", 125, HIGH, "Spotify", MixNotes(""));

    a.pushBack(p1);
    a.pushBack(p2);

    CHECK(a.getSize() == 2);
    CHECK(a.at(0) != nullptr);
    CHECK(a.at(1) != nullptr);
    CHECK(a.at(0)->getType() == "LocalTrack");
    CHECK(a.at(99) == nullptr);

    // Clean up: template doesn't delete owned objects
    delete p1;
    delete p2;
}

#endif
