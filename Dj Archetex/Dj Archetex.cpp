/*
DJ Set Architect (ITCS 2530 Starter Project)
Author: Yousif Triki

Purpose:
- Store DJ tracks in a small library (array of structs)
- View a formatted table summary
- Recommend next tracks based on BPM + energy rules
- Save a report to a text file

Concepts used (rubric):
- Constants (no magic numbers), enum, struct, array
- Input validation (chapter 3: cin fail states, clear/ignore)
- Switch menu, compound if/else conditions
- Loops: for, while, do-while
- User-defined functions
- Formatted output with setw + precision
- File output with ofstream
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

using namespace std;

// -------------------- Constants (avoid magic numbers) --------------------
const int MAX_TRACKS = 7;        // Maximum number of tracks stored
const int TITLE_W = 22;          // Column widths for table formatting
const int ARTIST_W = 18;
const int GENRE_W = 12;
const int KEY_W = 6;
const int NOTE_W = 20;
const int LINE_W = 78;

const int BPM_MIN = 60;          // Valid BPM range
const int BPM_MAX = 200;

const int MENU_MIN = 1;          // Valid menu choice range
const int MENU_MAX = 5;

// -------------------- Enum --------------------
// EnergyLevel models how intense a track feels in a set (meaningful for DJ planning).
enum EnergyLevel { LOW = 1, MEDIUM = 2, HIGH = 3 };

// -------------------- Struct --------------------
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

// Main features
void addTrack(Track library[], int& count);
void printLibrary(const Track library[], int count);
void recommendNextTracks(const Track library[], int count);
void saveReportToFile(const Track library[], int count, const string& filename);

// Derived values / calculations
double computeAverageBPM(const Track library[], int count);
int countGenreMatches(const Track library[], int count, const string& genre);

// Output helpers (reduces repeated table code)
void printTableHeader(ostream& out);
void printTrackRow(ostream& out, const Track& t);

// -------------------- Main --------------------
#ifndef _DEBUG
int main()
{
    // Array of structs to store multiple tracks (weekly/mini library style)
    Track library[MAX_TRACKS];
    int trackCount = 0; // how many slots are currently filled with valid tracks

    showBanner();

    // 3+ mixed user inputs: string (spaces), int, double
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
            cout << "\nGoodbye, " << djName << "! Keep the crowd moving.\n";
            break;
        default:
            // This should not happen due to validation, but it’s safe to include.
            cout << "Invalid choice.\n";
        }

    } while (choice != 5);

    return 0;
}
#endif

// -------------------- UI Functions --------------------

// Prints the program intro banner (friendly welcome).
void showBanner()
{
    cout << "=============================================\n";
    cout << "        DJ SET ARCHITECT - C++ EDITION       \n";
    cout << "   Plan smoother transitions and energy flow \n";
    cout << "=============================================\n";
}

// Prints the menu options for the user.
void showMenu()
{
    cout << "\n-------------------- MENU --------------------\n";
    cout << "1) Add a track to library\n";
    cout << "2) View library summary\n";
    cout << "3) Recommend next tracks (BPM/Energy rules)\n";
    cout << "4) Save report to file\n";
    cout << "5) Quit\n";
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
    if (e == LOW) return "LOW";
    if (e == MEDIUM) return "Medium";
    return "High";
}

// -------------------- Output Helpers (reuse for cout and file) --------------------

// Prints the table header to any output stream (cout or ofstream).
void printTableHeader(ostream& out)
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

// Prints one Track row to any output stream (cout or ofstream).
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

// -------------------- Main Features --------------------

// Adds a new track into the library array (if space is available).
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

// Prints the full library in a pretty table and shows derived values.
void printLibrary(const Track library[], int count)
{
    if (count == 0)
    {
        cout << "No tracks saved yet.\n";
        return;
    }

    cout << "\n==================== LIBRARY ====================\n";
    printTableHeader(cout);

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

// Recommends tracks that are close in BPM and keep energy steady/rising.
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
        // manual absolute value for bpm difference (stays within early C++ concepts)
        int bpmDiff = library[i].bpm - currentBPM;
        if (bpmDiff < 0) bpmDiff = -bpmDiff;

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

// Saves the formatted library report to a file.
void saveReportToFile(const Track library[], int count, const string& filename)
{
    ofstream fout(filename.c_str());
    if (!fout)
    {
        cout << "Could not open file: " << filename << "\n";
        return;
    }

    fout << "==================== DJ SET ARCHITECT REPORT ====================\n";
    fout << "Tracks stored: " << count << "\n\n";

    if (count == 0)
    {
        fout << "No tracks saved.\n";
        fout.close();
        cout << "Report saved to " << filename << "\n";
        return;
    }

    // Reuse the same formatting functions for file output
    printTableHeader(fout);
    for (int i = 0; i < count; i++)
        printTrackRow(fout, library[i]);

    // Save derived value (average BPM)
    double avg = computeAverageBPM(library, count);
    fout << "\nAverage BPM: " << fixed << setprecision(1) << avg << "\n";

    fout.close();
    cout << "Report saved to " << filename << "\n";
}

// -------------------- Calculations / Derived Values --------------------

// Computes the average BPM of stored tracks.
double computeAverageBPM(const Track library[], int count)
{
    if (count == 0) return 0.0;

    int sum = 0;
    for (int i = 0; i < count; i++)
        sum += library[i].bpm;

    return static_cast<double>(sum) / static_cast<double>(count);
}

// Counts how many tracks match a specific genre (exact match).
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

class DJLibrary
{
private:
    Track tracks[MAX_TRACKS];
    int count;

public:
    DJLibrary() : count(0) {}

    int getTrackCount() const { return count; }

    bool addTrackDirect(const Track& t)
    {
        // keep it simple: just prevent overflow
        if (count >= MAX_TRACKS) return false;
        tracks[count] = t;
        count++;
        return true;
    }
};

#ifdef _DEBUG

// Quick helper to make tracks easy in tests
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

// -------------------- A) Calculations (4 tests) --------------------

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

// -------------------- B) Enum decision logic (3 tests) --------------------

TEST_CASE("Energy enum: LOW prints 'Low'")
{
    CHECK(energyToString(LOW) == "Low");
}

TEST_CASE("Energy enum: MEDIUM prints 'Medium'")
{
    CHECK(energyToString(MEDIUM) == "Medium");
}

TEST_CASE("Energy enum: HIGH prints 'High'")
{
    CHECK(energyToString(HIGH) == "High");
}

// -------------------- C) Struct/array processing (3 tests) --------------------

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

// -------------------- D) Class methods (2 tests) --------------------

TEST_CASE("DJLibrary: addTrackDirect increases count")
{
    DJLibrary dj;
    CHECK(dj.getTrackCount() == 0);

    Track t = makeTrack("A", "House", 120, MEDIUM);
    CHECK(dj.addTrackDirect(t) == true);
    CHECK(dj.getTrackCount() == 1);
}

TEST_CASE("DJLibrary: cannot exceed MAX_TRACKS")
{
    DJLibrary dj;

    Track t = makeTrack("A", "House", 120, MEDIUM);
    for (int i = 0; i < MAX_TRACKS; i++)
        CHECK(dj.addTrackDirect(t) == true);

    CHECK(dj.getTrackCount() == MAX_TRACKS);
    CHECK(dj.addTrackDirect(t) == false); // overflow guard
}

#endif
