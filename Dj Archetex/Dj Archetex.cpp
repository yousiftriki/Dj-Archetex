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

// -------------------- Week 5/6/7: Manager Class --------------------
// TrackManager OWNS TrackBase* objects and uses DynamicArray<TrackBase*> for storage.
class TrackManager
{
private:
    DynamicArray<TrackBase*> items; // template-based dynamic container

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
    // Internal helper. We keep it throwing to match Week 07 behavior.
    void removeAt(int index)
    {
        // If invalid, DynamicArray::at throws out_of_range.
        TrackBase* doomed = items.at(index);
        delete doomed;
        items.removeAt(index);
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
    cout << "   Weeks 1-4 + Week 5/6/7 Upgrade Combined   \n";
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

    cout << "10) Quit\n";
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

#endif