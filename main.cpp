#include <raylib.h>
#include <raymath.h>
#include <cstring>
#include <cstdio>
#include <cctype>
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <cmath>
#include <string>
using namespace std;

static inline void DrawRoundedBorder(Rectangle rec, float roundness, int segments, float lineThick, Color color)
{
#if defined(RAYLIB_VERSION_MAJOR) || defined(RAYLIB_VERSION)
#ifdef DrawRectangleRoundedLines
#undef DrawRectangleRoundedLines
    ::DrawRectangleRoundedLines(rec, roundness, segments, lineThick, color);
#define DrawRectangleRoundedLines DrawRoundedBorder
#else
    ::DrawRectangleRoundedLines(rec, roundness, segments, color);
#endif
#else
    (void)roundness;
    (void)segments;
    DrawRectangleLinesEx(rec, lineThick, color);
#endif
}

#ifndef DRAW_ROUNDED_LINES_REMAP_DONE
#define DRAW_ROUNDED_LINES_REMAP_DONE
#define DrawRectangleRoundedLines DrawRoundedBorder
#endif

Color GetCourseColorByCredits(int credits)
{
    if (credits == 1)
        return Color{34, 197, 94, 255};
    else if (credits == 2)
        return Color{59, 130, 246, 255};
    else if (credits >= 3)
        return Color{168, 85, 247, 255};
    else
        return Color{148, 163, 184, 255};
}

string GetColorLegendText(int index)
{
    switch (index)
    {
    case 0:
        return "1 Credit";
    case 1:
        return "2 Credits";
    case 2:
        return "3 Credits";
    default:
        return "";
    }
}

class Student
{
public:
    int ID;
    string Name;
    string Email;
    string Phone;
    string Address;
    string Password;
};

class StudentNode
{
public:
    Student data;
    StudentNode *next;
};

class Course
{
public:
    int courseID;
    string courseName;
    int courseCredits;
    string courseInstructor;

    int maxCapacity;
    int currentEnrolled;

    static const int MAX_PREREQS = 10;
    int prereqIDs[MAX_PREREQS];
    int prereqCount;

    Course()
    {
        prereqCount = 0;
        for (int i = 0; i < MAX_PREREQS; i++)
        {
            prereqIDs[i] = -1;
        }
        maxCapacity = 0;
        currentEnrolled = 0;
    }
};

class CourseNode
{
public:
    Course data;
    CourseNode *left;
    CourseNode *right;
};

class Enrollment
{
public:
    int studentID;
    int courseID;
};

class EnrollmentNode
{
public:
    Enrollment data;
    EnrollmentNode *next;
    EnrollmentNode *prev;
};

class WaitlistItem
{
public:
    int studentID;
    int courseID;
};

class CourseHashNode
{
public:
    Course *data;
    CourseHashNode *next;
};

StudentNode *gStudentHead = NULL;
CourseNode *gCourseRoot = NULL;
EnrollmentNode *gEnrollmentHead = NULL;
EnrollmentNode *gEnrollmentTail = NULL;

static const int MAX_Q = 10;
WaitlistItem waitlistQ[MAX_Q];
int frontIdx = 0, rearIdx = -1, qCount = 0;

static const int TABLE_SIZE = 10;
CourseHashNode *courseTable[TABLE_SIZE];

static const int MAX_STACK = 10;
int prereqStack[MAX_STACK];
int stackTop = -1;

bool courseExists(int cID);
bool studentExists(int id);

Student *searchStudentByID(int id);
Course *searchCourseByID(int cID);

bool isStudentEnrolledInCourse(int studentID, int courseID);
bool meetsPrerequisites(int studentID, const Course &course);

void addEnrollment(int studentID, int courseID);
void viewEnrollment(int studentID);
bool validatePrerequisites(int courseID, int studentID);
bool enqueueWaitlist(int studentID, int courseID);
bool dequeueWaitlist();
void initCourseHashTable();
void insertCourseHash(Course *cPtr);
Course *searchCourseHash(int cID);
int rebuildHashFromBST(CourseNode *node);
bool deleteCourseHash(int cID);

void studentMenu();
void courseMenu();
void enrollmentMenu();
void prereqStackMenu();
void waitlistQueueMenu();
void hashTableMenu();

bool studentExists(int id)
{
    StudentNode *temp = gStudentHead;
    while (temp != NULL)
    {
        if (temp->data.ID == id)
        {
            return true;
        }
        temp = temp->next;
    }
    return false;
}

void addStudent(int id, const string &name, const string &email,
                const string &phone, const string &address, const string &password)
{
    if (studentExists(id))
    {
        cout << "Error: A student with ID " << id << " already exists.\n";
        return;
    }
    StudentNode *newNode = new StudentNode;
    newNode->data.ID = id;
    newNode->data.Name = name;
    newNode->data.Email = email;
    newNode->data.Phone = phone;
    newNode->data.Address = address;
    newNode->data.Password = password;
    newNode->next = NULL;

    if (gStudentHead == NULL)
    {
        gStudentHead = newNode;
        return;
    }
    StudentNode *temp = gStudentHead;
    while (temp->next != NULL)
    {
        temp = temp->next;
    }
    temp->next = newNode;
}

bool deleteStudent(int id)
{
    if (gStudentHead == NULL)
        return false;
    if (gStudentHead->data.ID == id)
    {
        StudentNode *toDelete = gStudentHead;
        gStudentHead = gStudentHead->next;
        delete toDelete;
        return true;
    }
    StudentNode *current = gStudentHead;
    while (current->next != NULL && current->next->data.ID != id)
    {
        current = current->next;
    }
    if (current->next == NULL)
    {
        return false;
    }
    StudentNode *toDelete = current->next;
    current->next = toDelete->next;
    delete toDelete;
    return true;
}

void displayStudents()
{
    cout << "\n-- Displaying All Students --\n";
    if (gStudentHead == NULL)
    {
        cout << "[No students found]\n";
        return;
    }
    StudentNode *temp = gStudentHead;
    while (temp != NULL)
    {
        cout << "Student ID: " << temp->data.ID << "\n"
             << "  Name: " << temp->data.Name << "\n"
             << "  Email: " << temp->data.Email << "\n"
             << "  Phone: " << temp->data.Phone << "\n"
             << "  Address: " << temp->data.Address << "\n"
             << "  Password: " << temp->data.Password << "\n"
             << "---------------------------------\n";
        temp = temp->next;
    }
    cout << endl;
}

Student *searchStudentByID(int id)
{
    StudentNode *temp = gStudentHead;
    while (temp != NULL)
    {
        if (temp->data.ID == id)
        {
            return &(temp->data);
        }
        temp = temp->next;
    }
    return NULL;
}

void sortStudentsByID()
{
    if (gStudentHead == NULL || gStudentHead->next == NULL)
        return;

    bool swapped;
    do
    {
        swapped = false;
        StudentNode *current = gStudentHead;
        while (current->next != NULL)
        {
            if (current->data.ID > current->next->data.ID)
            {
                Student temp = current->data;
                current->data = current->next->data;
                current->next->data = temp;
                swapped = true;
            }
            current = current->next;
        }
    } while (swapped);
}

class CourseNode;

CourseNode *insertCourseHelper(CourseNode *node, const Course &c)
{
    if (node == NULL)
    {
        CourseNode *newNode = new CourseNode;
        newNode->data = c;
        newNode->left = NULL;
        newNode->right = NULL;
        return newNode;
    }
    if (c.courseID < node->data.courseID)
    {
        node->left = insertCourseHelper(node->left, c);
    }
    else if (c.courseID > node->data.courseID)
    {
        node->right = insertCourseHelper(node->right, c);
    }
    return node;
}

bool courseExists(int cID)
{
    return (searchCourseByID(cID) != NULL);
}

CourseNode *searchCourseHelper(CourseNode *node, int cID)
{
    if (node == NULL)
        return NULL;
    if (node->data.courseID == cID)
        return node;
    if (cID < node->data.courseID)
    {
        return searchCourseHelper(node->left, cID);
    }
    else
    {
        return searchCourseHelper(node->right, cID);
    }
}

Course *searchCourseByID(int cID)
{
    Course *c = searchCourseHash(cID);
    if (c)
        return c;

    CourseNode *found = searchCourseHelper(gCourseRoot, cID);
    if (found)
        return &(found->data);

    return NULL;
}

CourseNode *insertCourseBST(const Course &c)
{
    if (courseExists(c.courseID))
    {
        return NULL;
    }

    gCourseRoot = insertCourseHelper(gCourseRoot, c);
    CourseNode *node = searchCourseHelper(gCourseRoot, c.courseID);

    if (node)
    {
        insertCourseHash(&node->data);
    }

    return node;
}

CourseNode *findMinCourseNode(CourseNode *node)
{
    while (node && node->left)
    {
        node = node->left;
    }
    return node;
}

CourseNode *dropCourseHelper(CourseNode *node, int cID)
{
    if (node == NULL)
        return NULL;
    if (cID < node->data.courseID)
    {
        node->left = dropCourseHelper(node->left, cID);
    }
    else if (cID > node->data.courseID)
    {
        node->right = dropCourseHelper(node->right, cID);
    }
    else
    {
        if (node->left == NULL)
        {
            CourseNode *tmp = node->right;
            delete node;
            return tmp;
        }
        else if (node->right == NULL)
        {
            CourseNode *tmp = node->left;
            delete node;
            return tmp;
        }
        else
        {
            CourseNode *minRight = findMinCourseNode(node->right);
            node->data = minRight->data;
            node->right = dropCourseHelper(node->right, minRight->data.courseID);
        }
    }
    return node;
}

void dropCourse(int cID)
{
    gCourseRoot = dropCourseHelper(gCourseRoot, cID);
    deleteCourseHash(cID);
}

void displayCoursesInOrderHelper(CourseNode *node)
{
    if (node == NULL)
        return;
    displayCoursesInOrderHelper(node->left);

    cout << "Course ID: " << node->data.courseID << "\n"
         << "  Name: " << node->data.courseName << "\n"
         << "  Credits: " << node->data.courseCredits << "\n"
         << "  Instructor: " << node->data.courseInstructor << "\n"
         << "  Capacity: " << node->data.currentEnrolled
         << "/" << node->data.maxCapacity << "\n";
    if (node->data.prereqCount > 0)
    {
        cout << "  Prerequisites: ";
        for (int i = 0; i < node->data.prereqCount; i++)
        {
            cout << node->data.prereqIDs[i];
            if (i < node->data.prereqCount - 1)
                cout << ", ";
        }
        cout << "\n";
    }
    cout << "---------------------------------\n";

    displayCoursesInOrderHelper(node->right);
}

void displayCoursesInOrder()
{
    cout << "\n-- Displaying All Courses (In-Order) --\n";
    if (!gCourseRoot)
    {
        cout << "[No courses found]\n\n";
        return;
    }
    displayCoursesInOrderHelper(gCourseRoot);
    cout << endl;
}

Course createCourseRecord(int cID);

Course createCourseRecord(int cID)
{
    Course c;
    c.courseID = cID;

    cout << "Enter Course Name: ";
    cin.ignore();
    getline(cin, c.courseName);

    cout << "Enter Credits: ";
    cin >> c.courseCredits;
    cin.ignore();

    cout << "Enter Instructor: ";
    getline(cin, c.courseInstructor);

    cout << "Enter Course Capacity (max students): ";
    cin >> c.maxCapacity;
    cin.ignore();
    c.currentEnrolled = 0;

    while (true)
    {
        cout << "\n-- Prerequisite Menu for Course " << cID << " --\n"
             << "1. Add a Prerequisite\n"
             << "2. Finish\n"
             << "Choice: ";
        int choice;
        cin >> choice;
        if (!cin)
        {
            cin.clear();
            cin.ignore(1000, '\n');
            continue;
        }
        if (choice == 2)
        {
            break;
        }
        else if (choice == 1)
        {
            if (c.prereqCount >= Course::MAX_PREREQS)
            {
                cout << "Max prereqs reached.\n";
                continue;
            }
            cout << "Enter prerequisite Course ID: ";
            int pID;
            cin >> pID;
            if (!courseExists(pID))
            {
                cout << "Prereq course " << pID << " doesn't exist, creating now...\n";
                Course newPrereq = createCourseRecord(pID);
                insertCourseBST(newPrereq);
            }
            c.prereqIDs[c.prereqCount] = pID;
            c.prereqCount++;
            cout << "Prerequisite " << pID << " added to course " << cID << ".\n";
        }
        else
        {
            cout << "[Invalid choice]\n";
        }
    }

    return c;
}
void addEnrollment(int studentID, int courseID)
{
    if (!searchStudentByID(studentID))
    {
        cout << "Error: Student " << studentID << " doesn't exist.\n";
        return;
    }
    if (!courseExists(courseID))
    {
        cout << "Error: Course " << courseID << " doesn't exist.\n";
        return;
    }
    if (!meetsPrerequisites(studentID, *searchCourseByID(courseID)))
    {
        cout << "Student " << studentID << "\n does not meet prerequisites for course " << courseID << ".\n";
        return;
    }

    Course *coursePtr = searchCourseByID(courseID);
    if (coursePtr->maxCapacity > 0 && coursePtr->currentEnrolled >= coursePtr->maxCapacity)
    {
        cout << "Course " << courseID << " is full ("
             << coursePtr->currentEnrolled << "/" << coursePtr->maxCapacity
             << "). Add student " << studentID << " to the waitlist instead.\n";
        return;
    }

    EnrollmentNode *checkPtr = gEnrollmentHead;
    while (checkPtr)
    {
        if (checkPtr->data.studentID == studentID &&
            checkPtr->data.courseID == courseID)
        {
            cout << "Error: Student " << studentID
                 << " already enrolled in " << courseID << ".\n";
            return;
        }
        checkPtr = checkPtr->next;
    }

    EnrollmentNode *newNode = new EnrollmentNode;
    newNode->data.studentID = studentID;
    newNode->data.courseID = courseID;
    newNode->next = NULL;
    newNode->prev = NULL;

    if (!gEnrollmentHead)
    {
        gEnrollmentHead = gEnrollmentTail = newNode;
    }
    else
    {
        gEnrollmentTail->next = newNode;
        newNode->prev = gEnrollmentTail;
        gEnrollmentTail = newNode;
    }
    cout << "Enrollment added (student " << studentID
         << " in course " << courseID << ").\n";

    coursePtr->currentEnrolled++;
    cout << "Enrollment added (student " << studentID
         << " in course " << courseID << ").\n";
}

bool removeEnrollment(int studentID, int courseID)
{
    EnrollmentNode *cur = gEnrollmentHead;
    while (cur)
    {
        if (cur->data.studentID == studentID && cur->data.courseID == courseID)
        {
            if (cur->prev)
                cur->prev->next = cur->next;
            else
                gEnrollmentHead = cur->next;
            if (cur->next)
                cur->next->prev = cur->prev;
            else
                gEnrollmentTail = cur->prev;

            Course *c = searchCourseByID(courseID);
            if (c && c->currentEnrolled > 0)
                c->currentEnrolled--;

            delete cur;
            cout << "Student " << studentID
                 << " unenrolled from course " << courseID << ".\n";

            return true;
        }
        cur = cur->next;
    }
    cout << "Enrollment not found.\n";
    return false;
}

void viewEnrollment(int studentID)
{
    cout << "\n-- Enrollment History for Student " << studentID << " --\n";
    EnrollmentNode *cur = gEnrollmentHead;
    bool found = false;
    while (cur)
    {
        if (cur->data.studentID == studentID)
        {
            cout << "  Course ID: " << cur->data.courseID << "\n";
            found = true;
        }
        cur = cur->next;
    }
    if (!found)
    {
        cout << "  [No enrollment records found]\n";
    }
    cout << "---------------------------------\n\n";
}

bool isStudentEnrolledInCourse(int studentID, int courseID)
{
    EnrollmentNode *cur = gEnrollmentHead;
    while (cur)
    {
        if (cur->data.studentID == studentID &&
            cur->data.courseID == courseID)
        {
            return true;
        }
        cur = cur->next;
    }
    return false;
}

bool pushPrereq(int val)
{
    if (stackTop >= MAX_STACK - 1)
        return false;
    stackTop++;
    prereqStack[stackTop] = val;
    return true;
}
bool popPrereq()
{
    if (stackTop < 0)
        return false;
    stackTop--;
    return true;
}
int topPrereq()
{
    if (stackTop < 0)
        return -1;
    return prereqStack[stackTop];
}
bool isPrereqEmpty()
{
    return (stackTop < 0);
}

bool checkPrerequisitesWithStack(int studentID, const Course &course, bool verbose = false)
{
    stackTop = -1;

    for (int i = 0; i < course.prereqCount; ++i)
    {
        if (!pushPrereq(course.prereqIDs[i]))
        {
            if (verbose)
                cout << "[STACK] Overflow while pushing prereqs.\n";
            return false;
        }
    }

    while (!isPrereqEmpty())
    {
        int needed = topPrereq();
        popPrereq();

        if (!isStudentEnrolledInCourse(studentID, needed))
        {
            if (verbose)
            {
                cout << " -> Missing prerequisite: " << needed << "\n";
            }
            return false;
        }

        Course *pc = searchCourseByID(needed);
        if (pc)
        {
            for (int j = 0; j < pc->prereqCount; ++j)
            {
                if (!pushPrereq(pc->prereqIDs[j]))
                {
                    if (verbose)
                        cout << "[STACK] Overflow while pushing nested prereqs.\n";
                    return false;
                }
            }
        }
    }

    if (verbose)
    {
        cout << " -> All prerequisites met. Registration allowed!\n";
    }
    return true;
}

bool meetsPrerequisites(int studentID, const Course &course)
{
    return checkPrerequisitesWithStack(studentID, course, false);
}

bool validatePrerequisites(int courseID, int studentID)
{
    Student *s = searchStudentByID(studentID);
    if (!s)
    {
        cout << "Error: Student " << studentID << " doesn't exist.\n";
        return false;
    }

    Course *c = searchCourseByID(courseID);
    if (!c)
    {
        cout << "Error: Course " << courseID << " doesn't exist.\n";
        return false;
    }

    cout << "[STACK] Checking prerequisites for course "
         << courseID << ", student " << studentID << "...\n";

    return checkPrerequisitesWithStack(studentID, *c, true);
}

bool enqueueWaitlist(int studentID, int courseID)
{
    if (!searchStudentByID(studentID))
    {
        cout << "Student doesn't exist.\n";
        return false;
    }
    if (!courseExists(courseID))
    {
        cout << "Course doesn't exist.\n";
        return false;
    }
    for (int i = 0; i < qCount; i++)
    {
        int idx = (frontIdx + i) % MAX_Q;
        if (waitlistQ[idx].studentID == studentID &&
            waitlistQ[idx].courseID == courseID)
        {
            cout << "Already waitlisted.\n";
            return false;
        }
    }
    if (qCount == MAX_Q)
    {
        cout << "Waitlist full.\n";
        return false;
    }
    rearIdx = (rearIdx + 1) % MAX_Q;
    waitlistQ[rearIdx].studentID = studentID;
    waitlistQ[rearIdx].courseID = courseID;
    qCount++;
    cout << "Student " << studentID << " waitlisted for course " << courseID << ".\n";
    return true;
}

bool dequeueWaitlist()
{
    if (qCount == 0)
    {
        return false;
    }

    WaitlistItem w = waitlistQ[frontIdx];

    Course *c = searchCourseByID(w.courseID);
    if (!c)
    {
        frontIdx = (frontIdx + 1) % MAX_Q;
        qCount--;
        cout << "Removed invalid waitlist entry (course missing) for student "
             << w.studentID << ".\n";
        return true;
    }

    if (c->maxCapacity > 0 && c->currentEnrolled >= c->maxCapacity)
    {
        cout << "Course " << w.courseID << " is still full ("
             << c->currentEnrolled << "/" << c->maxCapacity
             << "). Waitlist remains unchanged.\n";
        return false;
    }

    if (!meetsPrerequisites(w.studentID, *c))
    {
        cout << "Student " << w.studentID
             << " does not meet prerequisites for course " << w.courseID
             << ". Waitlist remains unchanged.\n";
        return false;
    }

    frontIdx = (frontIdx + 1) % MAX_Q;
    qCount--;

    cout << "Enrolling student " << w.studentID
         << " from waitlist in course " << w.courseID << ".\n";
    addEnrollment(w.studentID, w.courseID);
    return true;
}

void initCourseHashTable()
{
    for (int i = 0; i < TABLE_SIZE; i++)
    {
        courseTable[i] = NULL;
    }
}
int hashFunction(int key)
{
    return key % TABLE_SIZE;
}
void insertCourseHash(Course *cPtr)
{
    if (!cPtr)
        return;

    int idx = hashFunction(cPtr->courseID);
    CourseHashNode *newNode = new CourseHashNode;
    newNode->data = cPtr;
    newNode->next = courseTable[idx];
    courseTable[idx] = newNode;
}

int rebuildHashFromBST(CourseNode *node)
{
    if (!node)
        return 0;

    int count = 0;

    count += rebuildHashFromBST(node->left);

    insertCourseHash(&node->data);

    count += rebuildHashFromBST(node->right);

    return count + 1;
}

Course *searchCourseHash(int cID)
{
    int idx = hashFunction(cID);
    CourseHashNode *cur = courseTable[idx];
    while (cur)
    {
        if (cur->data && cur->data->courseID == cID)
            return cur->data;
        cur = cur->next;
    }
    return NULL;
}

bool deleteCourseHash(int cID)
{
    int idx = hashFunction(cID);
    CourseHashNode *cur = courseTable[idx];
    CourseHashNode *prev = NULL;

    while (cur)
    {
        if (cur->data && cur->data->courseID == cID)
        {
            if (prev)
                prev->next = cur->next;
            else
                courseTable[idx] = cur->next;
            delete cur;
            return true;
        }
        prev = cur;
        cur = cur->next;
    }
    return false;
}

void studentMenu()
{
    while (true)
    {
        cout << "\n*** STUDENT MENU ***\n"
             << "1. Add Student\n"
             << "2. Delete Student\n"
             << "3. Display All Students\n"
             << "4. Sort Students by ID\n"
             << "5. Search Student by ID\n"
             << "0. Return\n"
             << "Choice: ";
        int ch;
        cin >> ch;
        if (!cin)
        {
            cin.clear();
            cin.ignore(1000, '\n');
            continue;
        }

        if (ch == 0)
        {
            break;
        }
        else if (ch == 1)
        {
            int id;
            string name, email, phone, addr, pass;
            cout << "Enter ID: ";
            cin >> id;
            cin.ignore();
            cout << "Name: ";
            getline(cin, name);
            cout << "Email: ";
            getline(cin, email);
            cout << "Phone: ";
            getline(cin, phone);
            cout << "Address: ";
            getline(cin, addr);
            cout << "Password: ";
            getline(cin, pass);

            addStudent(id, name, email, phone, addr, pass);
        }
        else if (ch == 2)
        {
            int id;
            cout << "Enter ID to delete: ";
            cin >> id;
            bool ok = deleteStudent(id);
            if (ok)
                cout << "Deleted.\n";
            else
                cout << "Not found.\n";
        }
        else if (ch == 3)
        {
            displayStudents();
        }
        else if (ch == 4)
        {
            sortStudentsByID();
            cout << "Sorted.\n";
        }
        else if (ch == 5)
        {
            int id;
            cout << "Enter ID to search: ";
            cin >> id;
            Student *s = searchStudentByID(id);
            if (s)
            {
                cout << "Found:\n"
                     << "  ID: " << s->ID << "\n"
                     << "  Name: " << s->Name << "\n"
                     << "  Email: " << s->Email << "\n"
                     << "  Phone: " << s->Phone << "\n"
                     << "  Address: " << s->Address << "\n"
                     << "  Password: " << s->Password << "\n";
            }
            else
            {
                cout << "Not found.\n";
            }
        }
        else
        {
            cout << "[Invalid choice]\n";
        }
    }
}

void courseMenu()
{
    while (true)
    {
        cout << "\n*** COURSE MENU ***\n"
             << "1. Add Course\n"
             << "2. Drop Course\n"
             << "3. Display Courses (In-Order)\n"
             << "4. Search Course by ID\n"
             << "0. Return\n"
             << "Choice: ";
        int ch;
        cin >> ch;
        if (!cin)
        {
            cin.clear();
            cin.ignore(1000, '\n');
            continue;
        }

        if (ch == 0)
        {
            break;
        }
        else if (ch == 1)
        {
            cout << "Enter Course ID: ";
            int cID;
            cin >> cID;
            if (courseExists(cID))
            {
                cout << "Course " << cID << " already exists.\n";
                continue;
            }
            Course c = createCourseRecord(cID);
            insertCourseBST(c);
            cout << "Course " << cID << " added with any prerequisites.\n";
        }
        else if (ch == 2)
        {
            int cID;
            cout << "Enter Course ID to drop: ";
            cin >> cID;
            dropCourse(cID);
            cout << "Dropped if existed.\n";
        }
        else if (ch == 3)
        {
            displayCoursesInOrder();
        }
        else if (ch == 4)
        {
            int cID;
            cout << "Enter ID: ";
            cin >> cID;
            Course *c = searchCourseByID(cID);
            if (c)
            {
                cout << "\n-- Course Found --\n"
                     << " ID: " << c->courseID << "\n"
                     << " Name: " << c->courseName << "\n"
                     << " Credits: " << c->courseCredits << "\n"
                     << " Instructor: " << c->courseInstructor << "\n";
                if (c->prereqCount > 0)
                {
                    cout << " Prereqs: ";
                    for (int i = 0; i < c->prereqCount; i++)
                    {
                        cout << c->prereqIDs[i] << (i < c->prereqCount - 1 ? ", " : "");
                    }
                }
                cout << "\n\n";
            }
            else
            {
                cout << "Not found.\n";
            }
        }
        else
        {
            cout << "[Invalid choice]\n";
        }
    }
}

void enrollmentMenu()
{
    while (true)
    {
        cout << "\n*** ENROLLMENT MENU ***\n"
             << "1. Add Enrollment\n"
             << "2. View Enrollment by Student\n"
             << "3. Remove Enrollment (Unenroll)\n"
             << "0. Return\n"
             << "Choice: ";
        int ch;
        cin >> ch;
        if (!cin)
        {
            cin.clear();
            cin.ignore(1000, '\n');
            continue;
        }

        if (ch == 0)
        {
            break;
        }
        else if (ch == 1)
        {
            int sID, cID;
            cout << "Student ID: ";
            cin >> sID;
            cout << "Course ID: ";
            cin >> cID;
            addEnrollment(sID, cID);
        }
        else if (ch == 2)
        {
            int sID;
            cout << "Student ID: ";
            cin >> sID;
            viewEnrollment(sID);
        }
        else if (ch == 3)
        {
            int sID, cID;
            cout << "Student ID: ";
            cin >> sID;
            cout << "Course ID: ";
            cin >> cID;
            removeEnrollment(sID, cID);
        }

        else
        {
            cout << "[Invalid choice]\n";
        }
    }
}

void prereqStackMenu()
{
    while (true)
    {
        cout << "\n*** PREREQ STACK MENU ***\n"
             << "1. Validate (course, student)\n"
             << "0. Return\n"
             << "Choice: ";
        int ch;
        cin >> ch;
        if (!cin)
        {
            cin.clear();
            cin.ignore(1000, '\n');
            continue;
        }
        if (ch == 0)
        {
            break;
        }
        else if (ch == 1)
        {
            int cID, sID;
            cout << "Course ID: ";
            cin >> cID;
            cout << "Student ID: ";
            cin >> sID;
            validatePrerequisites(cID, sID);
        }
        else
        {
            cout << "[Invalid choice]\n";
        }
    }
}

void waitlistQueueMenu()
{
    while (true)
    {
        cout << "\n*** WAITLIST MENU ***\n"
             << "1. Enqueue (student, course)\n"
             << "2. Dequeue\n"
             << "0. Return\n"
             << "Choice: ";
        int ch;
        cin >> ch;
        if (!cin)
        {
            cin.clear();
            cin.ignore(1000, '\n');
            continue;
        }
        if (ch == 0)
        {
            break;
        }
        else if (ch == 1)
        {
            int sID, cID;
            cout << "Student ID: ";
            cin >> sID;
            cout << "Course ID: ";
            cin >> cID;
            enqueueWaitlist(sID, cID);
        }
        else if (ch == 2)
        {
            if (!dequeueWaitlist())
            {
                cout << "Waitlist empty.\n";
            }
        }
        else
        {
            cout << "[Invalid choice]\n";
        }
    }
}

void hashTableMenu()
{
    while (true)
    {
        cout << "\n*** COURSE HASH TABLE MENU ***\n"
             << "1. Init Hash Table (Clear)\n"
             << "2. Rebuild Hash from BST\n"
             << "3. Search Course in Hash\n"
             << "0. Return\n"
             << "Choice: ";
        int ch;
        cin >> ch;
        if (!cin)
        {
            cin.clear();
            cin.ignore(1000, '\n');
            continue;
        }
        if (ch == 0)
        {
            break;
        }
        else if (ch == 1)
        {
            initCourseHashTable();
            cout << "Hash table cleared/initialized.\n";
        }
        else if (ch == 2)
        {
            initCourseHashTable();
            int cnt = rebuildHashFromBST(gCourseRoot);
            cout << "Hash table rebuilt from BST. " << cnt << " courses indexed.\n";
        }
        else if (ch == 3)
        {
            int cID;
            cout << "Enter Course ID: ";
            cin >> cID;

            Course *c = searchCourseHash(cID);
            if (c)
            {
                cout << "\n-- Found in Hash --\n"
                     << " ID: " << c->courseID << "\n"
                     << " Name: " << c->courseName << "\n"
                     << " Credits: " << c->courseCredits << "\n"
                     << " Instructor: " << c->courseInstructor << "\n";
                if (c->prereqCount > 0)
                {
                    cout << " Prereqs: ";
                    for (int i = 0; i < c->prereqCount; i++)
                    {
                        cout << c->prereqIDs[i];
                        if (i < c->prereqCount - 1)
                            cout << ", ";
                    }
                }
                cout << "\n\n";
            }
            else
            {
                cout << "Not found in hash.\n";
            }
        }
        else
        {
            cout << "[Invalid choice]\n";
        }
    }
}

enum ScreenID
{
    SCR_MAIN,
    SCR_STUDENTS,
    SCR_COURSES,
    SCR_ENROLL,
    SCR_PREREQ,
    SCR_WAITLIST,
    SCR_HASH,
    SCR_CONSOLE_PROMPT
};

int consoleMain()
{
    try
    {
        while (true)
        {
            cout << "\n=========== MAIN MENU ===========\n"
                 << "1. Manage Students (SLL)\n"
                 << "2. Manage Courses (BST)\n"
                 << "3. Manage Enrollments (DLL)\n"
                 << "4. Registration (Prereq Stack)\n"
                 << "5. Waitlist (Queue)\n"
                 << "6. Course Hash Table (Chaining)\n"
                 << "0. Exit\n"
                 << "=================================\n"
                 << "Enter your choice: ";
            int mainChoice;
            cin >> mainChoice;

            if (!cin)
            {
                cin.clear();
                cin.ignore(1000, '\n');
                continue;
            }

            if (mainChoice == 0)
            {
                cout << "Exiting program. Goodbye!\n";
                break;
            }
            else if (mainChoice == 1)
            {
                studentMenu();
            }
            else if (mainChoice == 2)
            {
                courseMenu();
            }
            else if (mainChoice == 3)
            {
                enrollmentMenu();
            }
            else if (mainChoice == 4)
            {
                prereqStackMenu();
            }
            else if (mainChoice == 5)
            {
                waitlistQueueMenu();
            }
            else if (mainChoice == 6)
            {
                hashTableMenu();
            }
            else
            {
                cout << "[Invalid choice]\n";
            }
        }
    }
    catch (int)
    {
        cout << "[ERROR] Integer exception.\n";
    }
    catch (...)
    {
        cout << "[ERROR] Unknown exception.\n";
    }

    return 0;
}

static const Color UI_BG = {15, 23, 42, 255};
static const Color UI_PANEL = {30, 41, 59, 255};
static const Color UI_ACCENT = {59, 130, 246, 255};
static const Color UI_ACCENT_D = {37, 99, 235, 255};
static const Color UI_TEXT = {241, 245, 249, 255};
static const Color UI_MUTED = {148, 163, 184, 255};
static const Color UI_SHADOW = {0, 0, 0, 120};
static const Color UI_SUCCESS = {34, 197, 94, 255};
static const Color UI_CARD = {51, 65, 85, 255};

static float UiScale()
{
    float sw = (float)GetScreenWidth();
    float sh = (float)GetScreenHeight();
    float s = fminf(sw / 1280.0f, sh / 820.0f);
    if (s < 0.85f)
        s = 0.85f;
    if (s > 1.35f)
        s = 1.35f;
    return s;
}

static int ScaleX(int baseX)
{
    float sw = (float)GetScreenWidth();
    return (int)(baseX * (sw / 1280.0f));
}

static int ScaleY(int baseY)
{
    float sh = (float)GetScreenHeight();
    return (int)(baseY * (sh / 820.0f));
}

static int ScaleSize(int baseSize)
{
    float s = UiScale();
    return (int)(baseSize * s);
}

static Rectangle ScaleRect(float baseX, float baseY, float baseW, float baseH)
{
    float sw = (float)GetScreenWidth();
    float sh = (float)GetScreenHeight();
    return {
        baseX * (sw / 1280.0f),
        baseY * (sh / 820.0f),
        baseW * (sw / 1280.0f),
        baseH * (sh / 820.0f)};
}

static Rectangle GetContentArea(int baseWidth)
{
    int sw = GetScreenWidth();
    int contentWidth = ScaleX(baseWidth);
    int startX = (sw - contentWidth) / 2;
    return {(float)startX, 0.0f, (float)contentWidth, 0.0f};
}

static Font gUIFont = {0};
static Font gUIFontItalic = {0};
static bool gHasCustomFont = false;
static bool gHasItalicFont = false;

static void DrawUIText(const char *txt, int x, int y, int px, Color col, bool italic = false, float spacing = 1.0f)
{
    float s = UiScale();
    float size = px * s;
    if (!gHasCustomFont)
    {
        DrawText(txt, x, y, (int)roundf(size), col);
        return;
    }

    const Font &f = italic ? gUIFontItalic : gUIFont;
    Vector2 pos = { (float)x, (float)y };
    DrawTextEx(f, txt, pos, size, spacing, col);
}


static int MeasureUIText(const char *txt, int px, bool italic = false, float spacing = 1.0f)
{
    float s = UiScale();
    float size = px * s;
    if (!gHasCustomFont)
        return MeasureText(txt, (int)roundf(size));
    const Font &f = italic ? gUIFontItalic : gUIFont;
    Vector2 m = MeasureTextEx(f, txt, size, spacing);
    return (int)roundf(m.x);
}

struct Button
{
    Rectangle r;
    const char *label;
    bool primary = true;
};

static bool DrawButton(const Button &b)
{
    Vector2 m = GetMousePosition();
    bool hover = CheckCollisionPointRec(m, b.r);
    bool down = hover && IsMouseButtonDown(MOUSE_LEFT_BUTTON);
    bool click = hover && IsMouseButtonReleased(MOUSE_LEFT_BUTTON);

    Rectangle sr = {b.r.x + 1, b.r.y + 3, b.r.width, b.r.height};
    DrawRectangleRounded(sr, 0.25f, 8, UI_SHADOW);

    Color bg = b.primary ? (hover ? UI_ACCENT_D : UI_ACCENT)
                         : (hover ? Color{71, 85, 105, 255} : UI_PANEL);
    if (down)
        bg = b.primary ? Color{29, 78, 216, 255} : Color{51, 65, 85, 255};

    DrawRectangleRounded(b.r, 0.25f, 8, bg);

    Color border = b.primary ? (hover ? Color{96, 165, 250, 180} : Color{59, 130, 246, 120})
                             : Color{71, 85, 105, 180};
    DrawRectangleRoundedLines(b.r, 0.25f, 8, 2, border);

    int baseFontSize = 20;
    int fontSize = ScaleSize(baseFontSize);
    int tw = MeasureText(b.label, fontSize);
    Color txt = UI_TEXT;

    DrawText(b.label,
             (int)(b.r.x + (b.r.width - tw) / 2),
             (int)(b.r.y + (b.r.height - fontSize) / 2),
             fontSize,
             txt);

    return click;
}

struct TextBox
{
    Rectangle r;
    string text;
    bool focused = false;
    int maxLen = 128;
    bool numericOnly = false;
    bool primary = true;
    int caretPos = 0;
    bool justFocused = false;
};

static void DrawTextBox(TextBox &tb, const char *placeholder = "")
{
    Vector2 m = GetMousePosition();
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        bool wasFocused = tb.focused;
        tb.focused = CheckCollisionPointRec(m, tb.r);
        tb.justFocused = tb.focused && !wasFocused;
        if (tb.justFocused)
            tb.caretPos = (int)tb.text.size();
    }

    Rectangle sr = {tb.r.x + 1, tb.r.y + 2, tb.r.width, tb.r.height};
    DrawRectangleRounded(sr, 0.2f, 6, Color{0, 0, 0, 80});
    Color bg = tb.focused ? Color{51, 65, 85, 255} : Color{30, 41, 59, 255};
    DrawRectangleRounded(tb.r, 0.2f, 6, bg);

    Color border = tb.focused ? UI_ACCENT : Color{71, 85, 105, 255};
    DrawRectangleRoundedLines(tb.r, 0.2f, 6, tb.focused ? 2 : 1, border);

    if (tb.focused)
    {
        int key = GetCharPressed();
        while (key > 0)
        {
            if ((key >= 32) && (key <= 125))
            {
                if (!tb.numericOnly || (key >= '0' && key <= '9'))
                {
                    if ((int)tb.text.size() < tb.maxLen)
                    {
                        tb.text.push_back((char)key);
                        tb.caretPos = (int)tb.text.size();
                    }
                }
            }
            key = GetCharPressed();
        }
        if (IsKeyPressed(KEY_BACKSPACE) && !tb.text.empty())
        {
            tb.text.pop_back();
            tb.caretPos = (int)tb.text.size();
        }
    }

    const bool empty = tb.text.empty();
    const char *show = empty ? placeholder : tb.text.c_str();
    const int basePx = 18;
    const int px = ScaleSize(basePx);
    const int xpad = ScaleSize(14);
    const int ypad = ScaleSize(10);

    if (empty)
    {
        DrawText(show, (int)tb.r.x + xpad, (int)tb.r.y + ypad, px, UI_MUTED);
    }
    else
    {
        DrawText(show, (int)tb.r.x + xpad, (int)tb.r.y + ypad, px, UI_TEXT);
    }

    if (tb.focused)
    {
        static float caretTimer = 0.0f;
        caretTimer += GetFrameTime();
        if (fmod(caretTimer, 1.0f) < 0.55f)
        {
            string pre = tb.text.substr(0, tb.caretPos);
            int cx = (int)tb.r.x + xpad + MeasureText(pre.c_str(), px);
            int cy = (int)tb.r.y + ypad;
            DrawRectangle(cx, cy, 2, px, UI_ACCENT);
        }
    }
}

static int toInt(const string &s)
{
    if (s.empty())
        return 0;
    try
    {
        return stoi(s);
    }
    catch (...)
    {
        return 0;
    }
}

static ScreenID current = SCR_MAIN;
static string toastMsg;
static float toastTimer = 0.0f;

static void ShowToast(const string &s)
{
    toastMsg = s;
    toastTimer = 2.0f;
}

static void DrawTopBar()
{
    int sw = GetScreenWidth();
    int barHeight = ScaleY(80);

    DrawRectangleGradientV(0, 0, sw, barHeight,
                           Color{30, 41, 59, 255},
                           Color{15, 23, 42, 255});

    DrawRectangle(0, barHeight - 2, sw, 2, UI_ACCENT);

    int titleSize = ScaleSize(26);
    int subtitleSize = ScaleSize(18);
    int margin = ScaleX(40);

    DrawText("UNIVERSITY", margin, ScaleY(20), titleSize, UI_TEXT);
    DrawText("Management System", margin, ScaleY(48), subtitleSize, UI_MUTED);

    Rectangle accent = ScaleRect(30, 20, 4, 44);
    DrawRectangleRounded(accent, 1.0f, 4, UI_ACCENT);
}

static void DrawToast()
{
    if (toastTimer > 0)
    {
        toastTimer -= GetFrameTime();
        int w = MeasureText(toastMsg.c_str(), 16) + 32;
        Rectangle r{(float)(GetScreenWidth() - w - 20), 100.0f, (float)w, 40.0f};
        Rectangle sr{r.x + 2, r.y + 3, r.width, r.height};
        DrawRectangleRounded(sr, 0.3f, 8, UI_SHADOW);
        DrawRectangleRounded(r, 0.3f, 8, UI_SUCCESS);
        DrawRectangleRoundedLines(r, 0.3f, 8, 2, Color{74, 222, 128, 255});
        DrawText(toastMsg.c_str(), (int)r.x + 16, (int)r.y + 12, 16, Color{255, 255, 255, 255});
    }
}

static void DrawBackground()
{
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), UI_BG);

    static float time = 0.0f;
    time += GetFrameTime() * 0.3f;

    int sw = GetScreenWidth();
    int sh = GetScreenHeight();

    float offset1 = sin(time) * 20;
    float offset2 = cos(time * 0.7f) * 15;

    DrawCircleGradient(sw - 150, 120 + (int)offset1, 280, Color{59, 130, 246, 25}, Color{15, 23, 42, 0});
    DrawCircleGradient(180, sh - 150 + (int)offset2, 250, Color{168, 85, 247, 20}, Color{15, 23, 42, 0});
    DrawCircleGradient(sw / 2, sh / 2, 180, Color{34, 197, 94, 10}, Color{15, 23, 42, 0});
}

static void ScreenStudents()
{
    DrawTopBar();

    Rectangle content = GetContentArea(900);
    int startX       = (int)content.x;
    int contentWidth = (int)content.width;

    DrawText("Student Management",
             startX,
             ScaleY(100),
             ScaleSize(24),
             UI_TEXT);
    DrawRectangle(startX,
                  ScaleY(130),
                  ScaleX(160),
                  ScaleY(3),
                  UI_ACCENT);

    Rectangle card = {
        (float)startX,
        (float)ScaleY(150),
        (float)contentWidth,
        (float)ScaleY(160)
    };
    DrawRectangleRounded(card, 0.02f, 8, UI_CARD);
    DrawRectangleRoundedLines(card, 0.02f, 8, 1.0f, Color{71, 85, 105, 255});

    static TextBox id, name, email, phone, addr, pass;

    id.numericOnly  = true;
    id.maxLen       = 16;
    id.r = {
        (float)(startX + ScaleX(20)),
        (float)ScaleY(170),
        (float)ScaleX(140),
        (float)ScaleY(38)
    };

    name.r = {
        (float)(startX + ScaleX(170)),
        (float)ScaleY(170),
        (float)ScaleX(200),
        (float)ScaleY(38)
    };

    email.r = {
        (float)(startX + ScaleX(380)),
        (float)ScaleY(170),
        (float)ScaleX(200),
        (float)ScaleY(38)
    };

    phone.r = {
        (float)(startX + ScaleX(590)),
        (float)ScaleY(170),
        (float)ScaleX(140),
        (float)ScaleY(38)
    };

    addr.r = {
        (float)(startX + ScaleX(20)),
        (float)ScaleY(228),
        (float)ScaleX(350),
        (float)ScaleY(38)
    };

    pass.r = {
        (float)(startX + ScaleX(380)),
        (float)ScaleY(228),
        (float)ScaleX(200),
        (float)ScaleY(38)
    };

    int labelSize = ScaleSize(14);

    DrawText("ID",
             startX + ScaleX(20),
             ScaleY(152),
             labelSize,
             UI_MUTED);
    DrawTextBox(id, "1001");

    DrawText("Name",
             startX + ScaleX(170),
             ScaleY(152),
             labelSize,
             UI_MUTED);
    DrawTextBox(name, "Full Name");

    DrawText("Email",
             startX + ScaleX(380),
             ScaleY(152),
             labelSize,
             UI_MUTED);
    DrawTextBox(email, "user@domain.com");

    DrawText("Phone",
             startX + ScaleX(590),
             ScaleY(152),
             labelSize,
             UI_MUTED);
    DrawTextBox(phone, "03xx-xxxxxxx");

    DrawText("Address",
             startX + ScaleX(20),
             ScaleY(210),
             labelSize,
             UI_MUTED);
    DrawTextBox(addr, "Street, City");

    DrawText("Password",
             startX + ScaleX(380),
             ScaleY(210),
             labelSize,
             UI_MUTED);
    DrawTextBox(pass, "********");

    Button addBtn = {
        {
            (float)(startX + ScaleX(750)),
            (float)ScaleY(228),
            (float)ScaleX(130),
            (float)ScaleY(38)
        },
        "Add Student"
    };

    if (DrawButton(addBtn))
    {
        if (id.text.empty() || name.text.empty())
        {
            ShowToast("ID & Name required");
        }
        else
        {
            addStudent(toInt(id.text), name.text, email.text, phone.text, addr.text, pass.text);
            id.text.clear();
            name.text.clear();
            email.text.clear();
            phone.text.clear();
            addr.text.clear();
            pass.text.clear();
            ShowToast("Student added successfully");
        }
    }

    Rectangle actCard = {
        (float)startX,
        (float)ScaleY(330),
        (float)contentWidth,
        (float)ScaleY(80)
    };
    DrawRectangleRounded(actCard, 0.02f, 8, UI_CARD);
    DrawRectangleRoundedLines(actCard, 0.02f, 8, 1.0f, Color{71, 85, 105, 255});

    static TextBox searchId;
    searchId.numericOnly = true;
    searchId.maxLen      = 16;
    searchId.r = {
        (float)(startX + ScaleX(20)),
        (float)ScaleY(350),
        (float)ScaleX(140),
        (float)ScaleY(38)
    };

    DrawText("Quick Actions",
             startX + ScaleX(20),
             ScaleY(332),
             labelSize,
             UI_MUTED);
    DrawTextBox(searchId, "Student ID");

    Button searchBtn = {
        {
            (float)(startX + ScaleX(170)),
            (float)ScaleY(350),
            (float)ScaleX(110),
            (float)ScaleY(38)
        },
        "Search",
        false
    };

    Button deleteBtn = {
        {
            (float)(startX + ScaleX(290)),
            (float)ScaleY(350),
            (float)ScaleX(110),
            (float)ScaleY(38)
        },
        "Delete",
        false
    };

    Button sortBtn = {
        {
            (float)(startX + ScaleX(410)),
            (float)ScaleY(350),
            (float)ScaleX(140),
            (float)ScaleY(38)
        },
        "Sort by ID",
        false
    };

    if (DrawButton(searchBtn))
    {
        Student *s = searchStudentByID(toInt(searchId.text));
        if (s)
            ShowToast(string("Found: ") + s->Name);
        else
            ShowToast("Student not found");
        searchId.text.clear();
    }

    if (DrawButton(deleteBtn))
    {
        bool ok = deleteStudent(toInt(searchId.text));
        ShowToast(ok ? "Student deleted" : "Not found");
        searchId.text.clear();
    }

    if (DrawButton(sortBtn))
    {
        sortStudentsByID();
        ShowToast("Students sorted");
        searchId.text.clear();
    }

    Rectangle tableCard = {
        (float)startX,
        (float)ScaleY(430),
        (float)contentWidth,
        (float)ScaleY(360)
    };
    DrawRectangleRounded(tableCard, 0.02f, 8, UI_CARD);
    DrawRectangleRoundedLines(tableCard, 0.02f, 8, 1.0f, Color{71, 85, 105, 255});

    DrawText("All Students",
             startX + ScaleX(20),
             ScaleY(445),
             ScaleSize(18),
             UI_TEXT);

    int tblY      = ScaleY(475);
    int rowHeight = ScaleY(30);

    DrawRectangle(startX + ScaleX(20),
                  tblY,
                  contentWidth - ScaleX(40),
                  rowHeight,
                  Color{51, 65, 85, 255});

    int headerSize = ScaleSize(15);
    DrawText("ID",    startX + ScaleX(30),  tblY + ScaleY(8), headerSize, UI_MUTED);
    DrawText("Name",  startX + ScaleX(100), tblY + ScaleY(8), headerSize, UI_MUTED);
    DrawText("Email", startX + ScaleX(350), tblY + ScaleY(8), headerSize, UI_MUTED);
    DrawText("Phone", startX + ScaleX(620), tblY + ScaleY(8), headerSize, UI_MUTED);

    tblY += ScaleY(35);
    int row = 0;
    StudentNode *cur = gStudentHead;
    int dataRowHeight = ScaleY(28);
    int dataSize      = ScaleSize(15);

    while (cur && row < 9)
    {
        Color rowBg = (row % 2 == 0) ? Color{30, 41, 59, 150} : Color{30, 41, 59, 50};
        DrawRectangle(startX + ScaleX(20),
                      tblY,
                      contentWidth - ScaleX(40),
                      dataRowHeight,
                      rowBg);

        DrawText(TextFormat("%d", cur->data.ID),
                 startX + ScaleX(30), tblY + ScaleY(6), dataSize, UI_TEXT);
        DrawText(cur->data.Name.c_str(),
                 startX + ScaleX(100), tblY + ScaleY(6), dataSize, UI_TEXT);
        DrawText(cur->data.Email.c_str(),
                 startX + ScaleX(350), tblY + ScaleY(6), dataSize, UI_TEXT);
        DrawText(cur->data.Phone.c_str(),
                 startX + ScaleX(620), tblY + ScaleY(6), dataSize, UI_TEXT);

        tblY += dataRowHeight;
        row++;
        cur = cur->next;
    }
}

static void ScreenCourses()
{
    DrawTopBar();

    Rectangle content = GetContentArea(900);
    int startX       = (int)content.x;
    int contentWidth = (int)content.width;

    DrawText("Course Management",
             startX,
             ScaleY(100),
             ScaleSize(24),
             UI_TEXT);
    DrawRectangle(startX,
                  ScaleY(130),
                  ScaleX(150),
                  ScaleY(3),
                  UI_ACCENT);

    Rectangle card = {
        (float)startX,
        (float)ScaleY(150),
        (float)contentWidth,
        (float)ScaleY(110)
    };
    DrawRectangleRounded(card, 0.02f, 8, UI_CARD);
    DrawRectangleRoundedLines(card, 0.02f, 8, 1.0f, Color{71, 85, 105, 255});

    static TextBox cid, cname, ccred, ccap, cinst;
    int labelSize = ScaleSize(14);

    cid.numericOnly  = true;
    cid.maxLen       = 16;
    cid.r = {
        (float)(startX + ScaleX(20)),
        (float)ScaleY(170),
        (float)ScaleX(120),
        (float)ScaleY(38)
    };

    cname.r = {
        (float)(startX + ScaleX(150)),
        (float)ScaleY(170),
        (float)ScaleX(240),
        (float)ScaleY(38)
    };

    ccred.numericOnly = true;
    ccred.maxLen      = 3;
    ccred.r = {
        (float)(startX + ScaleX(400)),
        (float)ScaleY(170),
        (float)ScaleX(90),
        (float)ScaleY(38)
    };

    ccap.numericOnly = true;
    ccap.maxLen      = 4;
    ccap.r = {
        (float)(startX + ScaleX(500)),
        (float)ScaleY(170),
        (float)ScaleX(90),
        (float)ScaleY(38)
    };

    cinst.r = {
        (float)(startX + ScaleX(600)),
        (float)ScaleY(170),
        (float)ScaleX(190),
        (float)ScaleY(38)
    };

    DrawText("ID",
             startX + ScaleX(20),
             ScaleY(152),
             labelSize,
             UI_MUTED);
    DrawTextBox(cid, "501");

    DrawText("Course Name",
             startX + ScaleX(150),
             ScaleY(152),
             labelSize,
             UI_MUTED);
    DrawTextBox(cname, "Data Structures");

    DrawText("Credits",
             startX + ScaleX(400),
             ScaleY(152),
             labelSize,
             UI_MUTED);
    DrawTextBox(ccred, "3");

    DrawText("Capacity",
             startX + ScaleX(500),
             ScaleY(152),
             labelSize,
             UI_MUTED);
    DrawTextBox(ccap, "30");

    DrawText("Instructor",
             startX + ScaleX(600),
             ScaleY(152),
             labelSize,
             UI_MUTED);
    DrawTextBox(cinst, "Prof. Khan");

    Button addBtn = {
        {
            (float)(startX + ScaleX(20)),
            (float)ScaleY(212),
            (float)ScaleX(130),
            (float)ScaleY(38)
        },
        "Add Course"
    };

    if (DrawButton(addBtn))
    {
        if (cid.text.empty() || cname.text.empty())
        {
            ShowToast("ID & Name required");
        }
        else
        {
            Course c;
            c.courseID        = toInt(cid.text);
            c.courseName      = cname.text;
            c.courseCredits   = toInt(ccred.text);
            c.courseInstructor = cinst.text;

            int cap        = toInt(ccap.text);
            c.maxCapacity  = (cap < 0) ? 0 : cap;
            c.currentEnrolled = 0;

            insertCourseBST(c);

            cid.text.clear();
            cname.text.clear();
            ccred.text.clear();
            ccap.text.clear();
            cinst.text.clear();

            ShowToast("Course added successfully");
        }
    }

    Rectangle actCard = {
        (float)startX,
        (float)ScaleY(270),
        (float)contentWidth,
        (float)ScaleY(80)
    };
    DrawRectangleRounded(actCard, 0.02f, 8, UI_CARD);
    DrawRectangleRoundedLines(actCard, 0.02f, 8, 1.0f, Color{71, 85, 105, 255});

    static TextBox scid;
    scid.numericOnly = true;
    scid.maxLen      = 16;
    scid.r = {
        (float)(startX + ScaleX(20)),
        (float)ScaleY(290),
        (float)ScaleX(140),
        (float)ScaleY(38)
    };

    DrawText("Quick Actions",
             startX + ScaleX(20),
             ScaleY(272),
             labelSize,
             UI_MUTED);
    DrawTextBox(scid, "Course ID");

    Button searchBtn = {
        {
            (float)(startX + ScaleX(170)),
            (float)ScaleY(290),
            (float)ScaleX(110),
            (float)ScaleY(38)
        },
        "Search",
        false
    };
    Button dropBtn = {
        {
            (float)(startX + ScaleX(290)),
            (float)ScaleY(290),
            (float)ScaleX(110),
            (float)ScaleY(38)
        },
        "Drop",
        false
    };

    if (DrawButton(searchBtn))
    {
        Course *c = searchCourseByID(toInt(scid.text));
        ShowToast(c ? (string("Found: ") + c->courseName) : "Course not found");
        scid.text.clear();
    }

    if (DrawButton(dropBtn))
    {
        dropCourse(toInt(scid.text));
        ShowToast("Course dropped");
        scid.text.clear();
    }

    Rectangle prereqCard = {
        (float)startX,
        (float)ScaleY(360),
        (float)contentWidth,
        (float)ScaleY(90)
    };
    DrawRectangleRounded(prereqCard, 0.02f, 8, UI_CARD);
    DrawRectangleRoundedLines(prereqCard, 0.02f, 8, 1.0f, Color{71, 85, 105, 255});

    DrawText("Manage Prerequisites",
             startX + ScaleX(20),
             ScaleY(370),
             ScaleSize(16),
             UI_TEXT);

    static TextBox pcid, ppID;
    pcid.numericOnly = true;
    pcid.maxLen      = 16;
    ppID.numericOnly = true;
    ppID.maxLen      = 16;

    pcid.r = {
        (float)(startX + ScaleX(20)),
        (float)ScaleY(410),
        (float)ScaleX(140),
        (float)ScaleY(32)
    };
    ppID.r = {
        (float)(startX + ScaleX(170)),
        (float)ScaleY(410),
        (float)ScaleX(140),
        (float)ScaleY(32)
    };

    DrawText("Course ID",
             startX + ScaleX(20),
             ScaleY(393),
             ScaleSize(13),
             UI_MUTED);
    DrawTextBox(pcid, "501");
    DrawText("Prereq ID",
             startX + ScaleX(170),
             ScaleY(393),
             ScaleSize(13),
             UI_MUTED);
    DrawTextBox(ppID, "101");

    Button addPreBtn = {
        {
            (float)(startX + ScaleX(330)),
            (float)ScaleY(410),
            (float)ScaleX(200),
            (float)ScaleY(32)
        },
        "Add Prerequisite",
        false
    };
    if (DrawButton(addPreBtn))
    {
        int courseID = toInt(pcid.text);
        int prereqID = toInt(ppID.text);

        Course *course = searchCourseByID(courseID);
        if (!course)
        {
            ShowToast("Course not found");
        }
        else if (courseID == prereqID)
        {
            ShowToast("Course cannot be its own prerequisite");
        }
        else
        {
            Course *prCourse = searchCourseByID(prereqID);
            if (!prCourse)
            {
                ShowToast("Prereq course must exist first");
            }
            else if (course->prereqCount >= Course::MAX_PREREQS)
            {
                ShowToast("Max prerequisites reached for this course");
            }
            else
            {
                bool already = false;
                for (int i = 0; i < course->prereqCount; ++i)
                {
                    if (course->prereqIDs[i] == prereqID)
                    {
                        already = true;
                        break;
                    }
                }

                if (already)
                    ShowToast("Prerequisite already added");
                else
                {
                    course->prereqIDs[course->prereqCount] = prereqID;
                    course->prereqCount++;
                    ShowToast("Prerequisite added");
                }
            }
        }

        pcid.text.clear();
        ppID.text.clear();
    }

    Rectangle cardsCard = {
        (float)startX,
        (float)ScaleY(470),
        (float)contentWidth,
        (float)ScaleY(400)
    };
    DrawRectangleRounded(cardsCard, 0.02f, 8, UI_CARD);
    DrawRectangleRoundedLines(cardsCard, 0.02f, 8, 1.0f, Color{71, 85, 105, 255});

    DrawText("All Courses (Color-Coded by Credits)",
             startX + ScaleX(20),
             ScaleY(485),
             ScaleSize(18),
             UI_TEXT);

    int legendY = ScaleY(515);
    DrawText("Color Legend:",
             startX + ScaleX(20),
             legendY,
             ScaleSize(13),
             UI_MUTED);

    Color legendColors[] = {
        Color{34, 197, 94, 255},
        Color{59, 130, 246, 255},
        Color{168, 85, 247, 255}
    };
    for (int i = 0; i < 3; i++)
    {
        int lx = startX + ScaleX(150 + i * 130);
        DrawRectangle(lx, legendY - 2, ScaleX(12), ScaleY(12), legendColors[i]);
        DrawText(GetColorLegendText(i).c_str(),
                 lx + ScaleX(18),
                 legendY - 4,
                 ScaleSize(12),
                 UI_MUTED);
    }

    int baseCardY  = ScaleY(545);
    int cardW      = ScaleX(250);
    int cardH      = ScaleY(120);
    int colsPerRow = 3;
    int cardGap    = ScaleX(15);

    vector<CourseNode*> stack;
    CourseNode *n = gCourseRoot;
    int shown = 0;

    while ((n || !stack.empty()) && shown < 9)
    {
        while (n)
        {
            stack.push_back(n);
            n = n->left;
        }
        n = stack.back();
        stack.pop_back();

        int row = shown / colsPerRow;
        int col = shown % colsPerRow;

        int cardX = startX + ScaleX(20) + col * (cardW + cardGap);
        int cardY = baseCardY + row * (cardH + cardGap);

        Color cardColor = GetCourseColorByCredits(n->data.courseCredits);

        Rectangle courseCard = {
            (float)cardX,
            (float)cardY,
            (float)cardW,
            (float)cardH
        };
        DrawRectangleRounded(courseCard, 0.08f, 8, UI_CARD);

        DrawRectangle(cardX, cardY, ScaleX(5), cardH, cardColor);
        DrawRectangleRoundedLines(courseCard, 0.08f, 8, 1.5f, cardColor);

        int textX = cardX + ScaleX(15);
        int textY = cardY + ScaleY(10);

        DrawText(TextFormat("ID: %d", n->data.courseID),
                 textX, textY, ScaleSize(13), cardColor);
        DrawText(TextFormat("Credits: %d", n->data.courseCredits),
                 cardX + cardW - ScaleX(120),
                 textY,
                 ScaleSize(13),
                 UI_MUTED);

        if (n->data.maxCapacity > 0)
            DrawText(TextFormat("Cap: %d/%d",
                                n->data.currentEnrolled,
                                n->data.maxCapacity),
                     cardX + cardW - ScaleX(140),
                     textY + ScaleY(18),
                     ScaleSize(12),
                     UI_MUTED);
        else
            DrawText("Cap: unlimited",
                     cardX + cardW - ScaleX(140),
                     textY + ScaleY(18),
                     ScaleSize(12),
                     UI_MUTED);

        string displayName = n->data.courseName;
        if (displayName.length() > 25)
            displayName = displayName.substr(0, 22) + "...";
        DrawText(displayName.c_str(),
                 textX,
                 textY + ScaleY(25),
                 ScaleSize(14),
                 UI_TEXT);

        string displayInstr = n->data.courseInstructor;
        if (displayInstr.length() > 25)
            displayInstr = displayInstr.substr(0, 22) + "...";
        DrawText("Instructor:",
                 textX,
                 textY + ScaleY(50),
                 ScaleSize(12),
                 UI_MUTED);
        DrawText(displayInstr.c_str(),
                 textX,
                 textY + ScaleY(65),
                 ScaleSize(12),
                 UI_TEXT);

        if (n->data.prereqCount > 0)
        {
            DrawText(TextFormat("Prerequisites: %d", n->data.prereqCount),
                     textX,
                     textY + ScaleY(80),
                     ScaleSize(11),
                     Color{255, 193, 7, 255});
        }

        shown++;
        n = n->right;
    }
}

static void ScreenEnroll()
{
    DrawTopBar();

    Rectangle content = GetContentArea(720);
    int startX       = (int)content.x;
    int contentWidth = (int)content.width;

    DrawText("Enrollment Management",
             startX, ScaleY(100), ScaleSize(24), UI_TEXT);
    DrawRectangle(startX, ScaleY(130), ScaleX(180), ScaleY(3), UI_ACCENT);

    Rectangle card = {
        (float)startX,
        (float)ScaleY(160),
        (float)contentWidth,
        (float)ScaleY(150)
    };
    DrawRectangleRounded(card, 0.02f, 8, UI_CARD);
    DrawRectangleRoundedLines(card, 0.02f, 8, 1.0f, Color{71, 85, 105, 255});

    DrawText("Enroll Student in Course",
             startX + ScaleX(20), ScaleY(175), ScaleSize(20), UI_TEXT);

    static TextBox sid, cid;
    sid.numericOnly = true; sid.maxLen = 16;
    cid.numericOnly = true; cid.maxLen = 16;

    sid.r = { (float)(startX + ScaleX(20)),  (float)ScaleY(235),
              (float)ScaleX(200),            (float)ScaleY(40) };
    cid.r = { (float)(startX + ScaleX(230)), (float)ScaleY(235),
              (float)ScaleX(200),            (float)ScaleY(40) };

    DrawText("Student ID", startX + ScaleX(20), ScaleY(210),
             ScaleSize(16), UI_MUTED);
    DrawTextBox(sid, "1001");

    DrawText("Course ID", startX + ScaleX(230), ScaleY(210),
             ScaleSize(16), UI_MUTED);
    DrawTextBox(cid, "501");

    Button enrollBtn = {
        { (float)(startX + ScaleX(450)), (float)ScaleY(235),
          (float)ScaleX(140),            (float)ScaleY(38) },
        "Enroll"
    };

    if (DrawButton(enrollBtn))
    {
        int s = toInt(sid.text);
        int c = toInt(cid.text);

        Course *coursePtr = searchCourseByID(c);

        if (!coursePtr)
        {
            ShowToast("Course not found");
        }
        else if (coursePtr->maxCapacity > 0 &&
                 coursePtr->currentEnrolled >= coursePtr->maxCapacity)
        {
            ShowToast(TextFormat("Course %d is full (%d/%d). Add student to waitlist.",
                                 c, coursePtr->currentEnrolled, coursePtr->maxCapacity));
        }
        else
        {
            addEnrollment(s, c);
            ShowToast("Enrollment processed");
        }

        sid.text.clear();
        cid.text.clear();
    }

    Rectangle viewCard = {
        (float)startX,
        (float)ScaleY(330),
        (float)contentWidth,
        (float)ScaleY(160)
    };
    DrawRectangleRounded(viewCard, 0.02f, 8, UI_CARD);
    DrawRectangleRoundedLines(viewCard, 0.02f, 8, 1.0f, Color{71, 85, 105, 255});

    DrawText("View / Drop Enrollment",
             startX + ScaleX(20), ScaleY(345),
             ScaleSize(20), UI_TEXT);

    static TextBox vsid;
    vsid.numericOnly = true; vsid.maxLen = 16;

    vsid.r = { (float)(startX + ScaleX(20)), (float)ScaleY(390),
               (float)ScaleX(180),           (float)ScaleY(38) };

    DrawText("Student ID (for history)",
             startX + ScaleX(20), ScaleY(368),
             ScaleSize(16), UI_MUTED);
    DrawTextBox(vsid, "1001");

    Button viewBtn = {
        { (float)(startX + ScaleX(210)), (float)ScaleY(388),
          (float)ScaleX(180),            (float)ScaleY(38) },
        "View (Console)", false
    };

    if (DrawButton(viewBtn))
    {
        viewEnrollment(toInt(vsid.text));
        ShowToast("Check console output");
        vsid.text.clear();
    }

    static TextBox dsid, dcid;
    dsid.numericOnly = true; dsid.maxLen = 16;
    dcid.numericOnly = true; dcid.maxLen = 16;

    dsid.r = { (float)(startX + ScaleX(20)),  (float)ScaleY(450),
               (float)ScaleX(180),            (float)ScaleY(38) };
    dcid.r = { (float)(startX + ScaleX(210)), (float)ScaleY(450),
               (float)ScaleX(180),            (float)ScaleY(38) };

    DrawText("Drop: Student ID",
             startX + ScaleX(20), ScaleY(432),
             ScaleSize(16), UI_MUTED);
    DrawTextBox(dsid, "1001");

    DrawText("Drop: Course ID",
             startX + ScaleX(210), ScaleY(432),
             ScaleSize(16), UI_MUTED);
    DrawTextBox(dcid, "501");

    Button dropBtn = {
        { (float)(startX + ScaleX(400)), (float)ScaleY(450),
          (float)ScaleX(140),            (float)ScaleY(38) },
        "Drop"
    };

    if (DrawButton(dropBtn))
    {
        bool ok = removeEnrollment(toInt(dsid.text), toInt(dcid.text));
        ShowToast(ok ? "Enrollment dropped" : "Enrollment not found");
        dsid.text.clear();
        dcid.text.clear();
    }

    Rectangle infoBox = {
        (float)startX,
        (float)ScaleY(500),
        (float)contentWidth,
        (float)ScaleY(110)
    };
    DrawRectangleRounded(infoBox, 0.02f, 8, Color{59, 130, 246, 30});
    DrawRectangleRoundedLines(infoBox, 0.02f, 8, 1.0f, Color{59, 150, 246, 100});

    DrawText("i", startX + ScaleX(20), ScaleY(530),
             ScaleSize(28), UI_ACCENT);
    DrawText("Enrollment History",
             startX + ScaleX(60), ScaleY(520),
             ScaleSize(17), UI_TEXT);
    DrawText("History is printed in the console window.",
             startX + ScaleX(60), ScaleY(545),
             ScaleSize(14), UI_MUTED);
    DrawText("Make sure to check the terminal.",
             startX + ScaleX(60), ScaleY(565),
             ScaleSize(14), UI_MUTED);
}

static void ScreenPrereq()
{
    DrawTopBar();

    Rectangle content = GetContentArea(640);
    int startX       = (int)content.x;
    int contentWidth = (int)content.width;

    DrawText("Prerequisite Validation",
             startX, ScaleY(100),
             ScaleSize(24), UI_TEXT);
    DrawRectangle(startX, ScaleY(130), ScaleX(180), ScaleY(3), UI_ACCENT);

    Rectangle card = {
        (float)startX,
        (float)ScaleY(160),
        (float)contentWidth,
        (float)ScaleY(140)
    };
    DrawRectangleRounded(card, 0.02f, 8, UI_CARD);
    DrawRectangleRoundedLines(card, 0.02f, 8, 1.0f, Color{71, 85, 105, 255});

    DrawText("Check Prerequisites",
             startX + ScaleX(20),
             ScaleY(175),
             ScaleSize(18),
             UI_TEXT);

    static TextBox cid, sid;
    cid.numericOnly = true; cid.maxLen = 16;
    sid.numericOnly = true; sid.maxLen = 16;

    cid.r = { (float)(startX + ScaleX(20)),  (float)ScaleY(225),
              (float)ScaleX(180),            (float)ScaleY(38) };
    sid.r = { (float)(startX + ScaleX(210)), (float)ScaleY(225),
              (float)ScaleX(180),            (float)ScaleY(38) };

    DrawText("Course ID",
             startX + ScaleX(20),
             ScaleY(205),
             ScaleSize(15),
             UI_MUTED);
    DrawTextBox(cid, "501");

    DrawText("Student ID",
             startX + ScaleX(210),
             ScaleY(205),
             ScaleSize(15),
             UI_MUTED);
    DrawTextBox(sid, "1001");

    Button validateBtn = {
        { (float)(startX + ScaleX(410)), (float)ScaleY(225),
          (float)ScaleX(170),            (float)ScaleY(38) },
        "Validate"
    };

    if (DrawButton(validateBtn))
    {
        validatePrerequisites(toInt(cid.text), toInt(sid.text));
        ShowToast("Check console for results");
        cid.text.clear();
        sid.text.clear();
    }

    Rectangle infoBox = {
        (float)startX,
        (float)ScaleY(320),
        (float)contentWidth,
        (float)ScaleY(70)
    };
    DrawRectangleRounded(infoBox, 0.02f, 8, Color{59, 130, 246, 30});
    DrawRectangleRoundedLines(infoBox, 0.02f, 8, 1.0f, Color{59, 130, 246, 100});

    DrawText("i",
             startX + ScaleX(20), ScaleY(340),
             ScaleSize(24), UI_ACCENT);
    DrawText("Uses a stack to recursively verify prerequisites.",
             startX + ScaleX(60), ScaleY(340),
             ScaleSize(14), UI_MUTED);
    DrawText("Full validation results appear in the console.",
             startX + ScaleX(60), ScaleY(360),
             ScaleSize(14), UI_MUTED);
}

static void ScreenWaitlist()
{
    DrawTopBar();

    Rectangle content = GetContentArea(640);
    int startX = (int)content.x;
    int contentWidth = (int)content.width;

    DrawText("Waitlist Management",
             startX, ScaleY(100),
             ScaleSize(24), UI_TEXT);
    DrawRectangle(startX, ScaleY(130), ScaleX(160), ScaleY(3), UI_ACCENT);

    Rectangle card = {
        (float)startX,
        (float)ScaleY(160),
        (float)contentWidth,
        (float)ScaleY(140)};
    DrawRectangleRounded(card, 0.02f, 8, UI_CARD);
    DrawRectangleRoundedLines(card, 0.02f, 8, 1.0f, Color{71, 85, 105, 255});

    DrawText("Waitlist Queue",
             startX + ScaleX(20), ScaleY(175),
             ScaleSize(19), UI_TEXT);

    static TextBox sid, cid;
    sid.numericOnly = true;
    sid.maxLen = 16;
    cid.numericOnly = true;
    cid.maxLen = 16;

    sid.r = {(float)(startX + ScaleX(20)), (float)ScaleY(225),
             (float)ScaleX(180), (float)ScaleY(38)};
    cid.r = {(float)(startX + ScaleX(210)), (float)ScaleY(225),
             (float)ScaleX(180), (float)ScaleY(38)};

    DrawText("Student ID",
             startX + ScaleX(20), ScaleY(207),
             ScaleSize(15), UI_MUTED);
    DrawTextBox(sid, "1001");

    DrawText("Course ID",
             startX + ScaleX(210), ScaleY(207),
             ScaleSize(15), UI_MUTED);
    DrawTextBox(cid, "501");

    Button addBtn = {
        {(float)(startX + ScaleX(410)), (float)ScaleY(225),
         (float)ScaleX(90), (float)ScaleY(38)},
        "Add"};
    if (DrawButton(addBtn))
    {
        enqueueWaitlist(toInt(sid.text), toInt(cid.text));
        ShowToast("Added to waitlist");
    }

    Button procBtn = {
        {(float)(startX + ScaleX(510)), (float)ScaleY(225),
         (float)ScaleX(90), (float)ScaleY(38)},
        "Process",
        false};
    if (DrawButton(procBtn))
    {
        bool ok = dequeueWaitlist();
        ShowToast(ok ? "Student enrolled" : "Waitlist empty");
    }

    Rectangle infoBox = {
        (float)startX,
        (float)ScaleY(320),
        (float)contentWidth,
        (float)ScaleY(70)};
    DrawRectangleRounded(infoBox, 0.02f, 8, Color{34, 197, 94, 30});
    DrawRectangleRoundedLines(infoBox, 0.02f, 8, 1.0f, Color{34, 197, 94, 100});

    DrawText("i",
             startX + ScaleX(20), ScaleY(340),
             ScaleSize(24), UI_SUCCESS);
    DrawText("Waitlist operates as a FIFO queue. Process dequeues the next student.",
             startX + ScaleX(60), ScaleY(340),
             ScaleSize(14), UI_MUTED);
    DrawText("Check console for detailed enrollment messages.",
             startX + ScaleX(60), ScaleY(360),
             ScaleSize(14), UI_MUTED);
}

static void ScreenHash()
{
    DrawTopBar();

    Rectangle content = GetContentArea(720);
    int startX = (int)content.x;
    int contentWidth = (int)content.width;

    DrawText("Course Hash Table",
             startX, ScaleY(100),
             ScaleSize(24), UI_TEXT);
    DrawRectangle(startX, ScaleY(130), ScaleX(150), ScaleY(3), UI_ACCENT);

    Rectangle card = {
        (float)startX,
        (float)ScaleY(160),
        (float)contentWidth,
        (float)ScaleY(200)};
    DrawRectangleRounded(card, 0.02f, 8, UI_CARD);
    DrawRectangleRoundedLines(card, 0.02f, 8, 1.0f, Color{71, 85, 105, 255});

    DrawText("Hash Table Operations",
             startX + ScaleX(20), ScaleY(175),
             ScaleSize(18), UI_TEXT);

    Button initBtn = {
        {(float)(startX + ScaleX(20)), (float)ScaleY(210),
         (float)ScaleX(200), (float)ScaleY(40)},
        "Initialize Hash",
        false};
    if (DrawButton(initBtn))
    {
        initCourseHashTable();
        ShowToast("Hash table initialized");
    }

    Button rebuildBtn = {
        {(float)(startX + ScaleX(230)), (float)ScaleY(210),
         (float)ScaleX(260), (float)ScaleY(40)},
        "Rebuild Hash"};
    if (DrawButton(rebuildBtn))
    {
        initCourseHashTable();
        int cnt = rebuildHashFromBST(gCourseRoot);
        ShowToast(TextFormat("Rebuilt hash (%d courses)", cnt));
    }

    static TextBox cid;
    cid.numericOnly = true;
    cid.maxLen = 16;
    cid.r = {(float)(startX + ScaleX(20)), (float)ScaleY(300),
             (float)ScaleX(200), (float)ScaleY(38)};

    DrawUIText("Search by ID",
               startX + ScaleX(20), ScaleY(270),
               ScaleSize(16), UI_MUTED);
    DrawTextBox(cid, "501");

    Button searchBtn = {
        {(float)(startX + ScaleX(230)), (float)ScaleY(300),
         (float)ScaleX(160), (float)ScaleY(38)},
        "Search Hash"};
    if (DrawButton(searchBtn))
    {
        Course *c = searchCourseHash(toInt(cid.text));
        ShowToast(c ? (string("Found: ") + c->courseName) : "Not found");
    }

    Rectangle infoBox = {
        (float)startX,
        (float)ScaleY(370),
        (float)contentWidth,
        (float)ScaleY(80)};
    DrawRectangleRounded(infoBox, 0.02f, 8, Color{168, 85, 247, 30});
    DrawRectangleRoundedLines(infoBox, 0.02f, 8, 1.0f, Color{168, 85, 247, 100});

    DrawText("i",
             startX + ScaleX(20), ScaleY(390),
             ScaleSize(24), Color{168, 85, 247, 255});
    DrawText("Hash Table Implementation",
             startX + ScaleX(60), ScaleY(380),
             ScaleSize(16), UI_TEXT);
    DrawText("Uses chaining collision resolution with a table size of 10.",
             startX + ScaleX(60), ScaleY(405),
             ScaleSize(14), UI_MUTED);
    DrawText("Courses are indexed as they are added; 'Rebuild Hash' syncs from the BST.",
             startX + ScaleX(60), ScaleY(425),
             ScaleSize(14), UI_MUTED);
}

static void ScreenMain()
{
    DrawTopBar();

    int sw = GetScreenWidth();
    int sh = GetScreenHeight();
    int centerX = sw / 2;
    int centerY = sh / 2;

    DrawText("Welcome to the", centerX - 80, centerY - 180, 20, UI_MUTED);
    DrawText("University Management System", centerX - 280, centerY - 150, 32, UI_TEXT);
    DrawRectangle(centerX - 80, centerY - 110, 160, 3, UI_ACCENT);

    int cardW = 280;
    int cardH = 70;
    int gap = 20;
    int startX = centerX - (cardW * 2 + gap) / 2;
    int startY = centerY - 50;

    Rectangle card1 = {(float)startX, (float)startY, (float)cardW, (float)cardH};
    DrawRectangleRounded(card1, 0.08f, 8, UI_CARD);
    DrawRectangleRoundedLines(card1, 0.08f, 8, 1.5f, Color{71, 85, 105, 255});
    if (DrawButton({{(float)startX + 15, (float)startY + 15, (float)(cardW - 30), (float)(cardH - 30)}, "Students"}))
        current = SCR_STUDENTS;

    Rectangle card2 = {(float)(startX + cardW + gap), (float)startY, (float)cardW, (float)cardH};
    DrawRectangleRounded(card2, 0.08f, 8, UI_CARD);
    DrawRectangleRoundedLines(card2, 0.08f, 8, 1.5f, Color{71, 85, 105, 255});
    if (DrawButton({{(float)(startX + cardW + gap + 15), (float)startY + 15, (float)(cardW - 30), (float)(cardH - 30)}, "Courses"}))
        current = SCR_COURSES;

    startY += cardH + gap;
    Rectangle card3 = {(float)startX, (float)startY, (float)cardW, (float)cardH};
    DrawRectangleRounded(card3, 0.08f, 8, UI_CARD);
    DrawRectangleRoundedLines(card3, 0.08f, 8, 1.5f, Color{71, 85, 105, 255});
    if (DrawButton({{(float)startX + 15, (float)startY + 15, (float)(cardW - 30), (float)(cardH - 30)}, "Enrollments"}))
        current = SCR_ENROLL;

    Rectangle card4 = {(float)(startX + cardW + gap), (float)startY, (float)cardW, (float)cardH};
    DrawRectangleRounded(card4, 0.08f, 8, UI_CARD);
    DrawRectangleRoundedLines(card4, 0.08f, 8, 1.5f, Color{71, 85, 105, 255});
    if (DrawButton({{(float)(startX + cardW + gap + 15), (float)startY + 15, (float)(cardW - 30), (float)(cardH - 30)}, "Prerequisites"}))
        current = SCR_PREREQ;

    startY += cardH + gap;
    Rectangle card5 = {(float)startX, (float)startY, (float)cardW, (float)cardH};
    DrawRectangleRounded(card5, 0.08f, 8, UI_CARD);
    DrawRectangleRoundedLines(card5, 0.08f, 8, 1.5f, Color{71, 85, 105, 255});
    if (DrawButton({{(float)startX + 15, (float)startY + 15, (float)(cardW - 30), (float)(cardH - 30)}, "Waitlist"}))
        current = SCR_WAITLIST;

    Rectangle card6 = {(float)(startX + cardW + gap), (float)startY, (float)cardW, (float)cardH};
    DrawRectangleRounded(card6, 0.08f, 8, UI_CARD);
    DrawRectangleRoundedLines(card6, 0.08f, 8, 1.5f, Color{71, 85, 105, 255});
    if (DrawButton({{(float)(startX + cardW + gap + 15), (float)startY + 15, (float)(cardW - 30), (float)(cardH - 30)}, "Course Hash"}))
        current = SCR_HASH;

    startY += cardH + gap + 20;
    Rectangle consoleCard = {(float)(centerX - cardW / 2), (float)startY, (float)cardW, (float)cardH};
    DrawRectangleRounded(consoleCard, 0.08f, 8, Color{71, 85, 105, 255});
    DrawRectangleRoundedLines(consoleCard, 0.08f, 8, 1.5f, Color{100, 116, 139, 255});
    if (DrawButton({{(float)(centerX - cardW / 2 + 15), (float)startY + 15, (float)(cardW - 30), (float)(cardH - 30)}, "Console Mode", false}))
    {
        consoleMain();
    }
}

int main()
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);

    initCourseHashTable();

    InitWindow(1280, 820, "University Management System");
    SetWindowMinSize(960, 640);

    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        BeginDrawing();
        DrawBackground();

        switch (current)
        {
        case SCR_MAIN:
            ScreenMain();
            break;
        case SCR_STUDENTS:
            ScreenStudents();
            break;
        case SCR_COURSES:
            ScreenCourses();
            break;
        case SCR_ENROLL:
            ScreenEnroll();
            break;
        case SCR_PREREQ:
            ScreenPrereq();
            break;
        case SCR_WAITLIST:
            ScreenWaitlist();
            break;
        case SCR_HASH:
            ScreenHash();
            break;
        default:
            ScreenMain();
            break;
        }

        if (current != SCR_MAIN)
        {
            int sw = GetScreenWidth();
            Rectangle backBtn = {(float)(sw - 130), 16.0f, 110.0f, 36.0f};

            Vector2 m = GetMousePosition();
            bool hover = CheckCollisionPointRec(m, backBtn);
            bool click = hover && IsMouseButtonReleased(MOUSE_LEFT_BUTTON);

            Color bg = hover ? Color{51, 65, 85, 255} : Color{30, 41, 59, 255};
            DrawRectangleRounded(backBtn, 0.25f, 8, bg);
            DrawRectangleRoundedLines(backBtn, 0.25f, 8, 1.5f, Color{71, 85, 105, 255});

            int tw = MeasureText("Back", 16);
            DrawText("Back", (int)(backBtn.x + (backBtn.width - tw) / 2), (int)(backBtn.y + 10), 16, UI_TEXT);

            if (click)
                current = SCR_MAIN;
        }

        DrawToast();
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
