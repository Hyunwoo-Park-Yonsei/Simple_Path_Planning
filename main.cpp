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
int LaneNumTransform(int num);

bool sortseccol(const vector<double>& t1, const vector<double>& t2)
{
    return t1[1] > t2[1];
}

float move_car(double *X,double *Y, double &steer, Sprite &racer, float &a, double &speed)
{


    double s = sqrt(22.5*22.5 + 35*35);
    *X += s*cos(a);
    *Y += s*sin(a);
    a = a + steer*M_PI/180.0;
    *X -= s*cos(a);
    *Y -= s*sin(a);

    racer.rotate(steer);
    float head_angle = a - atan(35.0/22.5);
    *X += speed*sin(head_angle);
    *Y -= speed*cos(head_angle);

    return head_angle;
}

vector <double> cartesian_to_frenet(vector <double> &pos, vector <double> &frenet_pos, double &frenet_s)
{

    frenet_pos[0] = (pos[0] - 400)* 2.7 /400;
    frenet_pos[1] = frenet_s+ (800 - pos[1]);


    return frenet_pos;
}

vector <double> frenet_to_cartesian(vector <double> &frenet_pos)
{
    vector <double> pos;

    pos.push_back(frenet_pos[0] * 400 / 2.7);
    pos.push_back(-frenet_pos[1]);

    return pos;
}


void everythingsgodown(double &RacerY, vector <vector <double>> &wps)
{
    RacerY += 0.05;
    for(int i = 0; i < wps.size(); i++)
    {
        wps[i][1] += 0.05;
    }
}


vector <vector <double>> right_path_init() {
    vector <vector <double>> right_path;

    for(double i= 0; i<0.86; i = i +0.01){
        right_path.push_back({i,800*pow((i-0.43),3) + 200*i + 75});
    }

    return right_path;
}

vector <vector <double>> left_path_init() {
    vector <vector <double>> left_path;

    for(double i= 0; i<0.86; i = i +0.01){
        left_path.push_back({-i,800*pow((i-0.43),3) + 200*i + 75});
    }
    return left_path;
}

vector <vector <double>> waypoints_init(){
    vector <vector <double>> waypoint;
    int t = LaneNumTransform(2);
    for(int i = 600; i > 0; i = i - 5){waypoint.push_back({t,i});}
    return waypoint;
}

double longitudinal_control(double RacerY){
    int ths = 430;
    float k = 0.0003;
    return k*(RacerY - ths) + 0.05;
}

double stanley_control(double RacerX, double RacerY, float head_angle, vector <double> closest_wp, double speed){
    double steer = 0;
    float k = 0.001;
    double position_error = -RacerX + closest_wp[0];
    double heading_error = closest_wp[2] - head_angle;
    double cte = atan2(k*position_error,speed);
//    cout << endl;
//    cout << "position error " << position_error << endl;
//    cout << "cte " << cte << endl;
    cout << "heading error " << heading_error << endl;
    cout << "head_angle " << head_angle << endl;
    cout << "closest_wp[2] " << closest_wp[2] << endl;
    cout << endl;

    steer = heading_error + cte;
    return steer;
}

vector <double> find_closest_wp(double RacerY,vector <vector <double>> waypoints){
    vector <double> closest_wp = {0,0,0};
    double min_dist = 600;
    double yaw;
    for(int i = 0; i < waypoints.size(); i++){
        if(waypoints[i][1] - RacerY < 0){
            if(closest_wp[0] == waypoints[i][0]){return closest_wp;}
            double yaw = M_PI/2 - atan2((closest_wp[1]-waypoints[i][1]),(closest_wp[0]-waypoints[i][0]));
            closest_wp[2] = -yaw;
            return closest_wp;
            }
        closest_wp[0] = waypoints[i][0];
        closest_wp[1] = waypoints[i][1];
    }
    return closest_wp;
}

int collision_check(double RacerX,double RacerY, double Obs1X,double Obs1Y,double Obs2X,double Obs2Y, int main_lane){
    vector <double> obstacleX = {Obs1X, Obs2X};
    vector <double> obstacleY = {Obs1Y, Obs2Y};
    if(main_lane > 2){
        cout << "just left !!!!!!!!!!!!!!!" << endl;
        return -1;

    }
    else if(main_lane < 2){
        cout << "just right!!!!!!!!!!!!!!!!!!!!1 " << endl;
        return 1;
    }
    for(int i= 0; i<2; i++){
            if(abs(obstacleX[i] - RacerX - 130) < 30 && abs(RacerY - obstacleY[i] -175) > 30 ){return -1;}
    }
    return 1;
}

int collision_check2(double RacerX,double RacerY, double Obs1X,double Obs1Y,double Obs2X,double Obs2Y,int main_lane){
    vector <double> obstacleX = {Obs1X, Obs2X};
    vector <double> obstacleY = {Obs1Y, Obs2Y};
    if(main_lane == 1){
        for(int i= 0; i<2; i++){
            if(obstacleX[i] != 340){return 1;}
        }
    }
    else if(main_lane == 3){
        for(int i= 0; i<2; i++){
            if(obstacleX[i] != 340){return -1;}
        }
    }
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


    float angle = atan(35.0/22.5);
    double steer = 0.00;
    double speed = 0.05;

    vector <double> road_center = {210,340,470,600,-1};
    double frenet_s = 0;

    vector <vector <double>> waypoints_left = left_path_init();
    vector <vector <double>> waypoints_right = right_path_init();
    vector <vector <double>> waypoints = waypoints_init();
    vector <vector <double>> avoid_wps;
    int state = 1;
    int main_lane = 2;
    float head_angle;
    vector <double> closest_wp;



    //Set racer and Obs pos
//	RacerX=SCREEN_WIDTH/2;
    // 340, 430
    RacerX = 340;
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

    //GameSound.play();
    //GameSound.setLoop(true);

    //game loop
    vector <double> pos = {RacerX,RacerY};
    vector <double> frenet_pos = {0,0};
    int dir = 0;
    double new_lane_posx = -1;
    double new_lane_posy = 0;


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
                        {if(RacerY>0){RacerY=RacerY-50;}}
                    if (event.key.code == sf::Keyboard::Down)
                        {if(RacerY<SCREEN_HEIGH-70){head_angle = -1.4;}}
                }
        }



        //Creat scrolling background
        Background.setPosition(0,BackgroundY1);
        Background1.setPosition(0,BackgroundY2);
        if (BackgroundY2>0)
        {
            BackgroundY1=0;
            BackgroundY2=BackgroundY1-500;
        }
        BackgroundY1+=0.05;
        BackgroundY2+=0.05;


        frenet_s += 0.05;


        //Set Obs LOOP
        if (Obs1Y>SCREEN_HEIGH)
            {Obs1Y=0;Obs1X=getRandomLane();score++;} else {Obs1Y=Obs1Y+gameSpeed;}
        if (Obs2Y>SCREEN_HEIGH)
            {Obs2Y=0;Obs2X=getRandomLane();score++;} else {Obs2Y=Obs2Y+gameSpeed;}
        if (Obs3Y>SCREEN_HEIGH)
            {Obs3Y=0;Obs3X=600;score++;} else {Obs3Y=Obs3Y+gameSpeed;}
        if (Obs4Y>SCREEN_HEIGH)
            {Obs4Y=0;Obs4X=600;score++;} else {Obs4Y=Obs4Y+gameSpeed;}




        frenet_pos[1] = frenet_s;
        pos[0] = RacerX;
        pos[1] = RacerY;



        head_angle = move_car(&RacerX,&RacerY, steer, Racer, angle, speed);
        speed = longitudinal_control(RacerY);

        frenet_pos = cartesian_to_frenet(pos,frenet_pos, frenet_s);
//        cout << "position in the simulation" << pos[0] << " " << pos[1] << endl;
//        cout << "frenet " << frenet_pos[0] << " " << frenet_pos[1] << endl;
//        cout << "last wp " << waypoints.back()[0] << " " << waypoints.back()[1] << endl;
//        cout << "first wp " << waypoints[0][0] << " " << waypoints[0][1] << endl;
//        // head angle is radian
//        cout << "head angle " << head_angle << endl;
//        cout << "steer " << steer << endl;
//        cout << "speed " << speed << endl;
        cout << " " << endl;




        closest_wp = find_closest_wp(RacerY,waypoints);
        steer = stanley_control(RacerX,RacerY, head_angle,closest_wp,speed);




        everythingsgodown(RacerY, waypoints);

        // erasing waypoint that go overs the screen

        if(waypoints.size() > 0){
            if(waypoints[0][1] > 600){
                waypoints.erase(waypoints.begin());}}





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






        //Clear and redraw position

        app.clear();

        CircleShape wp(5);

        app.draw(Background);
        app.draw(Background1);
        app.draw(Racer);
        app.draw(Obs1);
        app.draw(Obs2);
        app.draw(Obs3);
        app.draw(Obs4);

        app.draw(text);


        cout << "state  " << state << endl;
        cout << "main lane " << main_lane << endl;

        //RacerX 와 RacerY 는 왼쪽 모서리고, waypoint도 차량의 왼쪽 끝 모서리 기준이라 이를 그림으로 볼때 해석하기 쉽게 17.5,35 평행이동 시켜줌
        for(int i =0; i < waypoints.size(); i++){
            wp.setFillColor(sf::Color(50, 250, 50));
            wp.setPosition(waypoints[i][0]+ 20,waypoints[i][1]+35);
            app.draw(wp);
        }


        if(state ==1){
            //creating main lane's waypoints
            double lastwp = waypoints.back()[1];
            int to_the_center = 0;
            if(lastwp > 5){
                if(new_lane_posx == -1){
                    waypoints.push_back({LaneNumTransform(main_lane),0});
                }
                else{
                    waypoints.push_back({new_lane_posx,0});
                }
            }



            for(int i =0; i < waypoints_right.size(); i++){
                wp.setFillColor(sf::Color(50, 50, 250));
                vector <double> cartesian_pos;
                cartesian_pos = frenet_to_cartesian(waypoints_right[i]);
                wp.setPosition(cartesian_pos[0]+ 20+RacerX,cartesian_pos[1]+35+RacerY);
                app.draw(wp);

            }
            for(int i =0; i < waypoints_left.size(); i++){
                wp.setFillColor(sf::Color(50, 50, 250));
                vector <double> cartesian_pos;
                cartesian_pos = frenet_to_cartesian(waypoints_left[i]);
                wp.setPosition(cartesian_pos[0]+ 20+RacerX,cartesian_pos[1]+35+RacerY);
                app.draw(wp);

            }
            if(LaneNumTransform(main_lane) == Obs1X || LaneNumTransform(main_lane) == Obs2X ){
                state = 2;
                dir = collision_check(RacerX, RacerY,Obs1X,Obs1Y,Obs2X,Obs2Y,main_lane);
                main_lane += dir;
            }


//            else if(main_lane != 2){
//                to_the_center = collision_check2(RacerX,RacerY,Obs1X,Obs1Y,Obs2X,Obs2Y,main_lane);
//                if(to_the_center != 0){
//                    dir = to_the_center;
//                    if(to_the_center ==1){
//                        state =2;
//                        main_lane +=1;
//                        }
//                    else if(to_the_center == -1){
//                        state=2;
//                        main_lane -=1;
//                        }
//                }
//            }
        }
        else if(state == 2){
            if(dir > 0){
                vector <vector <double>> new_waypoints_right;
                for(int i =0; i < waypoints_right.size(); i++){
                    vector <double> cartesian_pos;
                    cartesian_pos = frenet_to_cartesian(waypoints_right[i]);
                    new_waypoints_right.push_back({cartesian_pos[0]+RacerX,cartesian_pos[1]+RacerY + 35});
                }


                waypoints = new_waypoints_right;
                new_lane_posx = new_waypoints_right.back()[0];
                new_lane_posy = new_waypoints_right.back()[1];
            }
            else{
                vector <vector <double>> new_waypoints_left;
                for(int i =0; i < waypoints_left.size(); i++){
                    vector <double> cartesian_pos;
                    cartesian_pos = frenet_to_cartesian(waypoints_left[i]);
                    new_waypoints_left.push_back({cartesian_pos[0] + RacerX, cartesian_pos[1]+RacerY +35});
                }
                new_lane_posx = new_waypoints_left.back()[0];
                new_lane_posy = new_waypoints_left.back()[1];
                waypoints = new_waypoints_left;
            }

            for(int i = 160; i > 0; i = i -5){
                waypoints.push_back({new_lane_posx,i});
            }
            state = 3;
        }
        else if (state ==3){
            cout << new_lane_posy << endl;
            double lastwp = waypoints.back()[1];
            if(lastwp > 5){
                waypoints.push_back({new_lane_posx,0});
            }
            new_lane_posy += 0.05;
            if(new_lane_posy > RacerX){
            state = 1;
            }


        }


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
int LaneNumTransform(int num)
{
    if(num == 1){return 210;}
    if(num == 2){return 340;}
    if(num == 3){return 470;}
    if(num == 4){return 600;}

}
random_device rdd;
mt19937 genn(rdd());
uniform_int_distribution<int> diss(-5,5);
int getRandomnumber()
    {
        int k = diss(genn);
        return k;
    }
