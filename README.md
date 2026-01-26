# Dj Archetex
Author: Yousif Triki
Course: ITCS 2530 – C++ Programming
Project Type: Starter Project / Console Application

📌 Project Description

DJ Set Architect is a C++ console program designed to help plan and organize DJ sets.
The program stores music tracks with important details such as BPM, key, genre, energy level, and mixing notes. It allows users to build a small library of tracks, view formatted summaries, generate simple recommendations, and save reports to a file.

This project demonstrates foundational C++ concepts including structs, enums, arrays, input validation, loops, formatted output, file handling, and debugging.

🚀 Features

✅ Add tracks to a library (title, artist, genre, key, BPM, energy, notes)

✅ Display a formatted table summary using setw

✅ Calculate derived values such as:

Average BPM

Number of tracks in a selected genre

✅ Recommend next tracks based on:

BPM compatibility

Energy progression rules

✅ Save a formatted report to a text file

✅ Input validation to prevent invalid entries

✅ Menu system using a switch statement

🧱 Concepts Demonstrated

Constants (no magic numbers)

Enum (EnergyLevel)

Struct (Track)

Array of Structs

Selection Statements

if / else

switch

Loops

for

while

do-while

User-Defined Functions

Formatted Output

setw

setprecision

File Output

ofstream

Debugger Usage

Breakpoints

Variable inspection

▶️ How to Run
Requirements

Visual Studio 2022 (or any C++ compiler that supports C++11+)

Windows OS recommended

Steps

Clone the repository from GitHub.

Open the project in Visual Studio.

Build the solution:

Build → Rebuild Solution

Run the program:

Press F5 or click Start Debugging.

Follow the on-screen menu instructions.

📂 Output Files

When option “Save report to file” is selected, the program creates:

DJ_Set_Report.txt


This file contains:

A formatted table of all tracks

The calculated average BPM

🛠 Sample Usage

Enter your DJ name, target BPM, and preparation time.

Add one or more tracks to the library.

View the formatted library summary.

Generate recommendations based on BPM and energy.

Save the report to a file.

🌱 Future Improvements

Search and filter tracks by BPM, genre, or key

Larger track storage

Smarter harmonic mixing logic

Setlist auto-generation

GUI version instead of console
