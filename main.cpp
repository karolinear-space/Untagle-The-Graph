#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <sstream>
#include<windows.h>
#include <chrono>
#include <SFML/Graphics.hpp>
using namespace sf;
using namespace std;
using namespace std::chrono;

string name = "UNTANGLE THE GRAPH";
int w = 1000;
int h = 800;
int level = 4;
int playing_stage = 0; // 0 - splash screen, 1 - currently solving, 2 - won/lost
bool last_won = true;
const int radius = w/50;
Vector2i offset(radius,radius);
Font my_beautiful_font;
string dots = "............................................................";
sf::RenderWindow window(sf::VideoMode(w, h), "Untangle The Graph");

class Node{
public:
    Color nodeColor = Color::White;
    Vector2i nodePosition;
    //constructor
    Node(Vector2i nodePosition): nodePosition(nodePosition) {};
};

class Graph{
public:
    vector<Node> Nodes;
    vector<pair<int, int>> Edges;
    bool CheckCross();
    friend void GeneratePlanarGraph();
    friend void DrawGraph();
};

Graph G; // the global graph we work with

double distance(Vector2i pos1, Vector2i pos2){
    return sqrt(pow((pos1.x-pos2.x),2) + pow((pos1.y-pos2.y),2));
}

void PrintLevel(){
    Text l;
    l.setFont(my_beautiful_font);
    stringstream ss;
    ss << level;
    string s;
    ss >> s;
    l.setString("LEVEL "+s);
    int k=s.size()+6;
    l.setCharacterSize(2*w/k);
    l.setFillColor(Color::White);
    l.setOrigin(0, 0);
    window.draw(l);
}

void YouLost(){
    last_won = false;
    PrintLevel();
    Text t;
    t.setFont(my_beautiful_font);
    t.setString(dots+"\nGAME OVER"+"\nPress enter to try again\n"+dots);
    t.setCharacterSize(min(w/10, h/10));
    t.setFillColor(Color::White);
    t.setOrigin(0, -h/2);
    window.draw(t);
}

void YouWon(){
    last_won = true;
    PrintLevel();
    Text t;
    t.setFont(my_beautiful_font);
    t.setString(dots+"\nCONGRATULATIONS"+"\non passing this level\nPress enter to continue\n"+dots);
    t.setCharacterSize(min(w/10, h/10));
    t.setFillColor(Color::White);
    t.setOrigin(0, -h/2);
    window.draw(t);
}

void SplashScreen(){
    Text Name;
    Name.setFont(my_beautiful_font);
    Name.setString(name);
    Name.setCharacterSize(2*w/(name.size()));
    Name.setFillColor(Color::White);
    Name.setOrigin(0, 0);
    window.draw(Name);
    Text t1,t2,t3,t4, t5;
    t1.setFont(my_beautiful_font); t2.setFont(my_beautiful_font); t3.setFont(my_beautiful_font); t4.setFont(my_beautiful_font); t5.setFont(my_beautiful_font);
    string text1 = "You have to make a graph planar by moving its nodes.";
    string text2 = "Press 1 to choose the node your cursor points to.";
    string text3 = "Press 2 to drop the node to the place your cursor points to.";
    string text4 = "Press Enter to submit your graph. If it no edges intersect -- you won.";
    string text5 = "Otherwise you can try that level again.";
    t1.setString(text1); t2.setString(text2); t3.setString(text3); t4.setString(text4); t5.setString(text5);
    int max_len_of_a_line=max(max(text1.size(), text2.size()), max(text3.size(), max(text4.size(), text5.size())));
    int letter_size=(2*w)/max_len_of_a_line;
    t1.setCharacterSize(letter_size); t2.setCharacterSize(letter_size); t3.setCharacterSize(letter_size); t4.setCharacterSize(letter_size); t5.setCharacterSize(letter_size);
    t1.setFillColor(Color::White);
    t2.setFillColor(Color::White);
    t3.setFillColor(Color::White);
    t4.setFillColor(Color::White);
    t5.setFillColor(Color::White);
    t1.setOrigin(0, -h/2);
    t2.setOrigin(0, -h/2-letter_size);
    t3.setOrigin(0, -h/2-2*letter_size);
    t4.setOrigin(0, -h/2-3*letter_size);
    t5.setOrigin(0, -h/2-4*letter_size);
    window.draw(t1);
    window.draw(t2);
    window.draw(t3);
    window.draw(t4);
    window.draw(t5);
}

void FinalStage(){
    if(G.CheckCross()){
        YouLost();
    }
    else{
        YouWon();
    }
}

bool OnSegment(Vector2i p, Vector2i q, Vector2i r){ // if 1,2,3 are collinear, does 2 belong to the 1-3 segment?
    if (q.x <= max(p.x, r.x) && q.x >= min(p.x, r.x) &&
        q.y <= max(p.y, r.y) && q.y >= min(p.y, r.y))
        return true;
    return false;
}

int Orientation(Vector2i p, Vector2i q, Vector2i r){
    int val = (q.y - p.y) * (r.x - q.x) -
              (q.x - p.x) * (r.y - q.y);
    if (val == 0) return 0;  // 3 points are collinear
    return (val > 0)? 1: 2;
}

bool CheckCross2Segments(Vector2i p1, Vector2i q1, Vector2i p2, Vector2i q2){
    int o1 = Orientation(p1, q1, p2);
    int o2 = Orientation(p1, q1, q2);
    int o3 = Orientation(p2, q2, p1);
    int o4 = Orientation(p2, q2, q1);

    if (o1 != o2 && o3 != o4)
        return true;

    if (o1==0 and OnSegment(p1, p2, q1)) return true;
    if (o2==0 and OnSegment(p1, q2, q1)) return true;
    if (o3==0 and OnSegment(p2, p1, q2)) return true;
    if (o4==0 and OnSegment(p2, q1, q2)) return true;
    return false;
}

bool Graph::CheckCross(){
    for(auto edge1: G.Edges){
        for(auto edge2: G.Edges){
            Vector2i p1 = Nodes[edge1.first].nodePosition;
            Vector2i q1 = Nodes[edge1.second].nodePosition;
            Vector2i p2 = Nodes[edge2.first].nodePosition;
            Vector2i q2 = Nodes[edge2.second].nodePosition;
            if(p1==p2 or p1==q2 or q1==p2 or q1==q2) continue;
            if(CheckCross2Segments(p1, q1, p2, q2)) return true;
        }
    }
    return false;
}

int IdentifyNode(Vector2i pos){
    double min_dist=w;
    int the_closest_node=level+1;
    for(int i=0; i<level; i++){
        double curr_dist = distance(pos, G.Nodes[i].nodePosition);
        if(curr_dist<min_dist){
            min_dist = curr_dist;
            the_closest_node = i;
        }
    }
    if(the_closest_node == level+1) {
        cout << "Couldn't find the closest node. Returning the 0-th";
        return 0;
    }
    return the_closest_node;
}

void MoveNode(int num, Vector2i pos){
    (G.Nodes[num]).nodePosition = pos;
}

void GeneratePlanarGraph(){
    G.Nodes.clear();
    int red = 0;
    int green = 80;
    int blue = 100;
    int step = 256/level;
    for(int i=0; i<level; i++){
        int x = rand()%(w-4*radius)+2*radius; // now we won't have circles on the edge of the screen (I hope so)
        int y = rand()%(h-4*radius)+2*radius;
        G.Nodes.emplace_back(Vector2i(x, y));
        red = (red+step)%(256-100)+50; // I exclude small rgb's so that we are safe from black nodes
        green = (green+2*step)%(256-100)+50;
        blue = (blue+3*step)%(256-100)+50;
        G.Nodes[i].nodeColor = Color(red, green, blue);
    }
    //building a planar graph//
    G.Edges.push_back(make_pair<int, int>(0,1));
    G.Edges.push_back(make_pair<int, int>(1,2));
    G.Edges.push_back(make_pair<int>(0, 2));
    for(int i=3; i<level; i++){
        pair<int, int> p1, p2, p3;
        p1.first = i-1; p1.second = i;
        p2.first = i-2; p2.second = i;
        p3.first = i-3; p3.second = i;
        if(rand()%100>3*level) G.Edges.push_back(p1); //"if" makes sure some edges are not drawn
        if(rand()%100>3*level) G.Edges.push_back(p2);
        if(rand()%100>3*level) G.Edges.push_back(p3);
    }
}

void DrawNode(Node N){
    CircleShape c(radius);
    c.setPosition(N.nodePosition.x, N.nodePosition.y);
    c.setFillColor(N.nodeColor);
    window.draw(c);
}

void DrawEdge(Node First, Node Second){ // a few extra lines are drawn to make the edge thicker
    Vertex line1[]={
            Vertex(Vector2f(First.nodePosition+offset)),
            Vertex(Vector2f(Second.nodePosition+offset))
    };
    window.draw(line1, 2, Lines);
    Vertex line2[]={
            Vertex(Vector2f(First.nodePosition+offset+Vector2i(1,1))),
            Vertex(Vector2f(Second.nodePosition+offset+Vector2i(1, 1)))
    };
    window.draw(line2, 2, Lines);
    Vertex line3[]={
            Vertex(Vector2f(First.nodePosition+offset-Vector2i(1,-1))),
            Vertex(Vector2f(Second.nodePosition+offset-Vector2i(1, -1)))
    };
    window.draw(line3, 2, Lines);
}

void DrawGraph(){
    for(int i=0; i<level; i++){
        DrawNode(G.Nodes[i]);
    }
    for(auto &edge: G.Edges){
        Node First = G.Nodes[edge.first];
        Node Second = G.Nodes[edge.second];
        DrawEdge(First, Second);
    }
}

int main()
{
    srand(time(0));
    GeneratePlanarGraph();
    bool choosing_where_to_move = false;
    int nodeNum = 0;

    if (!my_beautiful_font.loadFromFile("../pixel_font.ttf"))
    {
        exit(2);
    }
    else{
        cout << "Successfully downloaded the font."<<endl;
    }

    //Set the clock
    auto start = chrono::high_resolution_clock::now();
    chrono::seconds splash_duration = std::chrono::seconds(3);

    while (window.isOpen()){
        Vector2i pos = Mouse::getPosition(window)-offset;
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed) window.close();
            //splash screen//
            if(playing_stage==0) {
                auto stop = high_resolution_clock::now();
                auto duration = duration_cast<chrono::seconds>(stop - start);
                if(duration>splash_duration) playing_stage=1;
            }
            //changing your playing stage//
            if (event.type == Event::KeyPressed)
            {
                if (event.key.code == Keyboard::Enter and playing_stage==1)
                {
                    playing_stage = 2;
                }
                else if (event.key.code == Keyboard::Enter and playing_stage==2)
                {
                    if(last_won) level++;
                    GeneratePlanarGraph();
                    playing_stage = 1;
                }
            }
            //move a node//
            if(playing_stage==1){
                if (event.type == Event::KeyPressed)
                    if (event.key.code == Keyboard::Num1) {
                        choosing_where_to_move = true;
                        nodeNum = IdentifyNode(pos);
                    }
                if(event.type == Event::KeyPressed)
                    if (event.key.code == Keyboard::Num2 and choosing_where_to_move) {
                        MoveNode(nodeNum, pos);
                        choosing_where_to_move = false;
                    }
            }
            window.clear();
            if(playing_stage==1) DrawGraph();
            else if(playing_stage==2) FinalStage();
            else SplashScreen();
            window.display();
        }
    }
    return 0;
}