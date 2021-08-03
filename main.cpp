#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include<iostream>
#include<cstdlib>
#include<ctime>
#include <string>
#include <vector>
#include <unistd.h>
#include <time.h>
#include <cmath>
#include <algorithm>
#include <random>
#include <deque>


using namespace std;
using namespace sf;


//Init game const
    const double SCREEN_WIDTH=800;
    const double SCREEN_HEIGH=600;
    const int borderLeft=140;
    const int borderRight=660;
    const int racerWidth=40;
    const int racerHeight=70;
    string stringscore="";
    int score=0;
    double gameSpeed=0.02;

//Create Main Windows
RenderWindow app(VideoMode(SCREEN_WIDTH, SCREEN_HEIGH), "Car Racing");

//Creat random Number for game loop
int getRandomLane();
int getRandomnumber();
// Create gameover screen
int gameOver();


bool sortseccol(const vector<double>& t1, const vector<double>& t2)
{
    return t1[1] > t2[1];
}

void move_car(double *X,double *Y, double &steer, Sprite &racer, float &a, double &throttle)
{
    double s = sqrt(22.5*22.5 + 35*35);
    *X += s*cos(a);
    *Y += s*sin(a);
    a = a + steer*M_PI/180.0;
    *X -= s*cos(a);
    *Y -= s*sin(a);

    racer.rotate(steer);
    float head_angle = a - atan(35.0/22.5);
    *X += throttle*sin(head_angle);
    *Y -= throttle*cos(head_angle) - 0.1;
//    cout << "head angle" << head_angle <<endl;
//    cout << throttle*cos(head_angle) - 1.0 << endl;
//    cout << "rotate_car function!" << endl;
}



void lateral_control(int &t, double * steer, int &direction, double *X,double *Y, vector <double> &point, bool &rotating, int &state, int &state_change_time, double &velocity)
{
    if(*X - point[0] > 0){direction = -1;}
    else{direction = 1;}

    if(state == 1)
    {
        if(t - state_change_time < 1000)
        {
            if(t-state_change_time <500)
            {
                *steer = 0.035*direction*(t-state_change_time)/500;
            }
            else{*steer = 0.035*direction*(1000+state_change_time-t)/500;}

            return;
        }
        else
        {
            state = 2;
            return;
        }
    }
    if(state ==2)
    {
        if(abs(point[0]-*X) > 5)
        {
            *steer = 0;
            if(velocity*(point[0]-*X) < 0)
            {
                state = 5;
                state_change_time =t;
                return;
            }

            return;
        }
        else
        {
            state = 3;
            state_change_time = t;
            return;
        }
    }
    if(state == 3)
    {
        if(t - state_change_time < 1000)
        {
            if(t-state_change_time <500)
            {
                *steer = 0.035*direction*(t-state_change_time)/500;
            }
            else{*steer = 0.035*direction*(1000+state_change_time-t)/500;}

            return;
        }
        else
        {
            state = 4;
            return;
        }
    }
    if(state ==4)
    {
        if(abs(point[0]-*X) > 5)
        {
            state = 1;
            state_change_time = t;
            *steer = 0;
            return;
        }
        else{return;}
    }
    if(state ==5){
//        cout << "it is state 5 " << endl;
        if(t - state_change_time < 1000)
        {
            if(t-state_change_time <500)
            {
                *steer = 0.07*direction*(t-state_change_time)/500;
            }
            else{*steer = 0.07*direction*(1000+state_change_time-t)/500;}

            return;
        }
        else
        {
            state = 2;
            return;
        }

    }
}
//void waypoints(RenderWindow &app, int &k)
//{
//    CircleShape circle1(5);
//    circle1.setFillColor(sf::Color(50, 250, 50));
//    circle1.setPosition(400+17.5,k+35);
//    app.draw(circle1);
//}

void waypoint(vector <double> &point,vector <vector <double>> &v, vector <int> &center_road)
{
//    point[0] = 200;
//    point[1] = 430;
    vector <int> availability = {1,1,1,1};
    vector <double> nearest_car;
    for(int i = 0; i < 4; i++)
    {
//        cout << "!!" << endl;
        if(v[i][1] > 100)
        {
            nearest_car.push_back(v[i][0]);
//            cout << nearest_car.size() << endl;
        }

    }
    for(int i = 0; i<4; i++)
    {
        for(int j = 0; j < nearest_car.size(); j++)
        {
            if(abs(nearest_car[j] - center_road[i]) < 40)
            {
                    // a center road must have some distance to all nearest car
                    availability[i] *= 0;
            }
        }
    }
    int q = nearest_car.size();
//    cout << q << endl;
    for(int k = 0; k < q; k ++)
    {
        nearest_car.pop_back();
    }

    for(int i =0; i < 4; i++)
    {
        if(availability[i] == 1){point[0] = center_road[i];}
    }

}

void longitudinal_control(vector <double> &point, double *X,double *Y, double &throttle)
{

    throttle = 0.1 + (*Y - point[1])*0.0003;
}

void lane_change(deque <deque<double>> &dq, vector <double> road_center, int &new_lane, int &old_lane, vector <vector <double>> v){
    cout << "lane_change function" << endl;
    for(int i =0; i < 4; i++){
        cout << i << endl;
        int a = find(road_center.begin(), road_center.end(), v[i][0])-road_center.begin();
        if(a!=road_center.size()){road_center.erase(road_center.begin() + a);}
    }
    cout << "iteration finished" << endl;
    int dist = 800;
    for(int i = 0; i< road_center.size(); i++){
        if (dist > road_center[i] - old_lane && road_center[i] != -1){
            new_lane = road_center[i];
            dist = road_center[i] - old_lane;
        }
    }

    for(int i =15; i > -1; i--){
        dq[i][0] = old_lane + (new_lane-old_lane)*(16-i)/16;
        cout << "old " << old_lane << endl;
        cout << "new " << new_lane << endl;
    }
    old_lane = new_lane;
}

int main()
{



    //Init game music
    SoundBuffer gameSoundBuffer;
        gameSoundBuffer.loadFromFile("sound/game.wav");
    Sound GameSound;
        GameSound.setBuffer(gameSoundBuffer);

    Texture background, racer, obs1, obs2, obs3, obs4,gameover;
    //Init font
    Font myfont;
    myfont.loadFromFile("font/xirod.ttf");

    //Load all images
    if (!background.loadFromFile("cars/background.png")) return EXIT_FAILURE;
    if (!racer.loadFromFile("cars/racer.png")) return EXIT_FAILURE;
    if (!obs1.loadFromFile("cars/obs1.png")) return EXIT_FAILURE;
    if (!obs2.loadFromFile("cars/obs2.png")) return EXIT_FAILURE;
    if (!obs3.loadFromFile("cars/obs3.png")) return EXIT_FAILURE;
    if (!obs4.loadFromFile("cars/obs4.png")) return EXIT_FAILURE;

    //Create sprite
    Sprite Background(background),Background1(background),Racer(racer),Obs1(obs1),Obs2(obs2),Obs3(obs3),Obs4(obs4),Gameover(gameover);

    double RacerX, RacerY,Obs1X, Obs1Y,Obs2X, Obs2Y,Obs3X, Obs3Y,Obs4X, Obs4Y;


    vector <vector<double>> v;
    int t = 0;
    int direction = 0;
    vector <double> point = {210,430};
    int state_change_time = 0;

    float angle = atan(35.0/22.5);
    double steer = 0.00;
    bool rotating = false;
    int state = 4;
    double throttle = 0.10;
    deque <deque <double>> dq;
    for(int i = 0; i < 22; i++){
    dq.push_back({350, i*20+10});
    }
    vector <vector <double>> main_points;

    vector <double> road_center = {210,340,470,600,-1};
    int new_lane = road_center[1];
    int old_lane = road_center[1];
    bool lane_changed = false;





    //Set racer and Obs pos
	RacerX=SCREEN_WIDTH/2;
	RacerY=SCREEN_HEIGH-racerHeight - 100;


	Obs1X=getRandomLane();
//	Obs2X=getRandomLane();
//	Obs3X=getRandomLane();
//	Obs4X=getRandomLane();
//	Obs1X=600;
	Obs2X=getRandomLane();
	Obs3X=600;
	Obs4X=600;

    Obs1Y=0,Obs2Y=-150,Obs3Y=-300,Obs4Y=-450;
    double BackgroundY1=0;
    double BackgroundY2=-600;
    bool avoiding = false;
    GameSound.play();
    GameSound.setLoop(true);
    double RacerX_history;
    double lateral_velocity = 0;

    //game loop



    while (app.isOpen())
    {


        //Init and count score
        stringscore="SCORE:"+to_string(score);
        Text text(stringscore, myfont, 15);
        text.setPosition(5,0);

        //Set car position
        Racer.setPosition(RacerX,RacerY);
        Obs1.setPosition(Obs1X,Obs1Y);
//        Obs2.setPosition(Obs2X,Obs2Y);
//        Obs3.setPosition(Obs3X,Obs3Y);
//        Obs4.setPosition(Obs4X,Obs4Y);
//        Obs1.setPosition(600,Obs1Y);
        Obs2.setPosition(Obs2X,Obs2Y);
        Obs3.setPosition(600,Obs3Y);
        Obs4.setPosition(600,Obs4Y);


        //Creat scrolling background
        Background.setPosition(0,BackgroundY1);
        Background1.setPosition(0,BackgroundY2);
        if (BackgroundY2>0)
        {
            BackgroundY1=0;
            BackgroundY2=BackgroundY1-500;
        }
        BackgroundY1+=0.1;
        BackgroundY2+=0.1;

        //Set Obs LOOP
        if (Obs1Y>SCREEN_HEIGH)
            {Obs1Y=0;Obs1X=getRandomLane();score++;} else {Obs1Y=Obs1Y+gameSpeed;}
        if (Obs2Y>SCREEN_HEIGH)
            {Obs2Y=0;Obs2X=getRandomLane();score++;} else {Obs2Y=Obs2Y+gameSpeed;}
        if (Obs3Y>SCREEN_HEIGH)
            {Obs3Y=0;Obs3X=600;score++;} else {Obs3Y=Obs3Y+gameSpeed;}
        if (Obs4Y>SCREEN_HEIGH)
            {Obs4Y=0;Obs4X=600;score++;} else {Obs4Y=Obs4Y+gameSpeed;}




//        cout << Obs1X << Obs2X << Obs3X << Obs4X << endl;
        v.push_back({Obs1X,Obs1Y});
        v.push_back({Obs2X,Obs2Y});
        v.push_back({Obs3X,Obs3Y});
        v.push_back({Obs4X,Obs4Y});
        sort(v.begin(),v.end(),sortseccol);


//        waypoint(point, v, center_road);
        point[0] = dq[21][0];

        lateral_control(t, &steer, direction, & RacerX, & RacerY, point, rotating, state, state_change_time, lateral_velocity);
        longitudinal_control(point, & RacerX, & RacerY, throttle);


        RacerX_history = RacerX;
        move_car(&RacerX,&RacerY, steer, Racer, angle, throttle);
        lateral_velocity = -RacerX_history + RacerX;





        t++;
        Event event;
        while (app.pollEvent(event))
        {
            if (event.type == Event::Closed)
                app.close();
            if (event.type == sf::Event::KeyPressed)
                {
                    if (event.key.code == sf::Keyboard::Left)
                        {if(RacerX>borderLeft) {RacerX=RacerX-10;}}
                    if (event.key.code == sf::Keyboard::Right)
                        {if(RacerX<borderRight){RacerX=RacerX+10;}}
                    if (event.key.code == sf::Keyboard::Up)
                        {if(RacerY>0){RacerY=RacerY-10;}}
                    if (event.key.code == sf::Keyboard::Down)
                        {if(RacerY<SCREEN_HEIGH-70){RacerY=RacerY+10;}}
                }
        }


        //Check if accident happen
        if (((RacerX>=(Obs1X-45)) &&(RacerX<=(Obs1X+45)))&&((RacerY>=(Obs1Y)) &&(RacerY)<=(Obs1Y+70)))
            {
                cout << "1 " << Obs1X << " " << Obs1Y << " " << RacerX << " " << RacerY << endl;
                GameSound.stop();gameOver();
            };
        if (((RacerX>=(Obs2X-45)) &&(RacerX<=(Obs2X+45)))&&((RacerY>=(Obs2Y)) &&(RacerY)<=(Obs2Y+70)))
            {
                cout << "2 " << Obs2X << " " << Obs2Y << " " << RacerX << " " << RacerY << endl;
                GameSound.stop();gameOver();
            };
        if (((RacerX>=(Obs3X-45)) &&(RacerX<=(Obs3X+45)))&&((RacerY>=(Obs3Y)) &&(RacerY)<=(Obs3Y+70)))
            {
                cout << "3 " << Obs3X << " " << Obs3Y << " " << RacerX << " " << RacerY << endl;
                GameSound.stop();gameOver();
            };
        if (((RacerX>=(Obs4X-45)) &&(RacerX<=(Obs4X+45)))&&((RacerY>=(Obs4Y)) &&(RacerY)<=(Obs4Y+70)))
            {
                cout << "4 " << Obs4X << " " << Obs4Y << " " << RacerX << " " << RacerY << endl;
                GameSound.stop();gameOver();
            };

        sf::CircleShape shape(5);
        // set the shape color to green
        shape.setFillColor(sf::Color(50, 250, 50));
        shape.setPosition(point[0]+17.5,point[1]+35);


//        waypoints(app, waypoints_ylist[2]);
//        cout << waypoints_ylist[2] << endl;


        //Clear and redraw position

        app.clear();

        sf::CircleShape point(5);


        app.draw(Background);
        app.draw(Background1);
        app.draw(Racer);
        app.draw(Obs1);
        app.draw(Obs2);
        app.draw(Obs3);
        app.draw(Obs4);
        app.draw(shape);


        for(int i = 0; i < dq.size(); i++){
            dq[i][1] += 0.1;
        }


        for(int i =0; i < dq.size(); i++){

            point.setFillColor(sf::Color(50, 250, 50));
            point.setPosition(dq[i][0]+ 17.5,dq[i][1]+35);
            app.draw(point);
        }

        for(int i=0; i < 4; i++)
        {
            if(dq[5][0] - v[i][1] < 300 && abs(dq[5][0]-v[i][0]) < 70 && lane_changed == false){
                cout << "?" << endl;
                lane_change(dq,road_center, new_lane, old_lane, v);
                lane_changed = true;
            }
            else{lane_changed = false;}

            // re-making point
            if(dq[21][1]> 430)
            {
                dq.pop_back();
                dq.push_front({new_lane,0});
            }
        }
        v.pop_back();
        v.pop_back();
        v.pop_back();
        v.pop_back();

        app.draw(text);
        app.display();
    }
    return EXIT_SUCCESS;
}
int startGame()
{
    //TODO
}
//Game over
int gameOver()
{
    Texture gameover,troll;
        if (!gameover.loadFromFile("cars/over.png")) return EXIT_FAILURE;
        if(!troll.loadFromFile("cars/troll.png")) return EXIT_FAILURE;

    Sprite Gameover(gameover);
    Sprite Troll(troll);
    Troll.setPosition(10,350);
    SoundBuffer gameOver;
        gameOver.loadFromFile("sound/crash.wav");
    Sound GameOver;
        GameOver.setBuffer(gameOver);
        GameOver.play();
    while (app.isOpen())
    {
        Event event;
        while (app.pollEvent(event))
        {
            if (event.type == Event::Closed)
                app.close();
        }
        Font myfont;
            myfont.loadFromFile("font/xirod.ttf");
            stringscore="YOUR SCORE:"+to_string(score);
        Text text(stringscore, myfont, 30);
            text.setPosition(210,450);
        app.clear();
        app.draw(Gameover);
        app.draw(text);
        app.draw(Troll);
        app.display();



    }

}
random_device rd;
mt19937 gen(rd());
uniform_int_distribution<int> dis(0,3);
int getRandomLane()
    {
        int k = dis(gen);
        if(k == 0){return 210;}
        if(k == 1){return 340;}
        if(k == 2){return 470;}
        if(k == 3){return 600;}

    }
random_device rdd;
mt19937 genn(rdd());
uniform_int_distribution<int> diss(-5,5);
int getRandomnumber()
    {
        int k = diss(genn);
        return k;
    }
