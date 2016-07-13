// OpenGL Libraries
#include <GL/glut.h>
#include <GL/freeglut.h>

// Additional Libraries
#include <math.h>
#include <iostream>
#include <vector>
#include <string>

#include "SOIL.h"     //For loading textures

#define PI 3.14159265 //Delicious

#define CW 1.5f       //Card Width
#define CH 2.0f       //Card Height
#define RC 4.0f       //Carousel Radius

//Path to texture files
std::string tex_path = "./textures/";

//Declared early for global variable
struct card;

// Window ID
int winID;

//Camera Positioning
GLfloat camx, camy, camz, losx, losy, losz, upx, upy, upz;

static int CC = 0;         //card count
static GLfloat spin = 0.0; //for carousel rotation
static GLfloat sd = 1.0;   //for carousel rotation direction
bool deck[52];             //if !deck[i], card taken
std::vector<card> cards;   //vector of card structs

enum prefixes{ACE = 0,TWO = 1,THREE = 2,FOUR = 3,FIVE = 4,
              SIX = 5,SEVEN = 6,EIGHT = 7,NINE = 8,TEN = 9,
              JACK = 10,QUEEN = 11,KING = 12};
enum suits{DIAMONDS = 1,CLUBS = 2,HEARTS = 3,SPADES = 4};

std::string pre2str(prefixes pre)
{
  std::string prefix = "_of_";
  switch (pre)
  {
    case 0: prefix = "ace" + prefix; break;
    case 1: prefix = "2" + prefix; break;
    case 2: prefix = "3" + prefix; break;
    case 3: prefix = "4" + prefix; break;
    case 4: prefix = "5" + prefix; break;
    case 5: prefix = "6" + prefix; break;
    case 6: prefix = "7" + prefix; break;
    case 7: prefix = "8" + prefix; break;
    case 8: prefix = "9" + prefix; break;
    case 9: prefix = "10" + prefix; break;
    case 10: prefix = "jack" + prefix; break;
    case 11: prefix = "queen" + prefix; break;
    case 12: prefix = "king" + prefix; break;
  }
  return prefix;
}

std::string st2str(suits s)
{
  std::string suit = "";
  switch (s)
  {
      case 1: suit += "diamonds"; break;
      case 2: suit += "clubs"; break;
      case 3: suit += "hearts"; break;
      case 4: suit += "spades"; break;
  }
  return suit;
}

typedef struct point
{
  GLfloat x,y,z;
  point(GLfloat x, GLfloat y, GLfloat z) : x(x), y(y), z(z) {}
  point() {}
  void set(GLfloat x1, GLfloat y1, GLfloat z1)
  {
    x = x1;
    y = y1;
    z = z1;
  }
} point;

typedef struct card
{
  prefixes prefix;
  suits suit;
  int index;
  int id;
  std::string tfname;
  point tr,tl,bl,br,center;
  GLfloat theta;
  GLuint texture;

  card(int ind) : index (ind)
  {
    if(ind > 51 || ind < 0 || !deck[ind]) //is the card valid?
        return;

    int lb = 0; //lower bound, each boundary set based on known order of
                //suits in un-shuffled deck
    tfname += tex_path; //path to texture file

    if(ind < 13)
    {
      suit = DIAMONDS;
      lb = 0;
    }
    else if(ind < 26)
    {
      suit = CLUBS;
      lb = 13;
    }
    else if(ind < 39)
    {
      suit = HEARTS;
      lb = 26;
    }
    else
    {
      suit = SPADES;
      lb = 39;
    }

    prefix = (prefixes)(ind - lb);

    tfname += pre2str(prefix);
    tfname += st2str(suit);
    tfname += ".png";

    //now that the texture file is known, load the texture
    texture = SOIL_load_OGL_texture
      (
        tfname.c_str(),
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y
      );
    if(texture == 0)
        std::cout<<"SOIL ERROR"<<std::endl;

    deck[ind] = false; // card taken, not available
    CC++;
    id = CC;
  }

  void getPoints(GLfloat theta_)
  {
    GLfloat cx,cz;
    if(theta_ > 360)
      theta_ -= 360;
    if(theta_ < 0)
      theta_ += 360;
    theta = theta_;
    cx = (GLfloat)(cos(theta*PI/180) * RC);
    cz = (GLfloat)(sin(theta*PI/180) * RC);

    tr.set(cx + (CW/2), CH/2, cz);
    tl.set(cx - (CW/2), CH/2, cz);
    bl.set(cx - (CW/2), -(CH/2), cz);
    br.set(cx + (CW/2), -(CH/2), cz);

    center.set(cx, 0.0f, cz);
  }

  void render()
  {
    glTranslatef(center.x, center.y, center.z);
    glRotatef(-spin, 0.0, 1.0, 0.0);
    glTranslatef(-1*center.x, -1*center.y, -1*center.z);
    //draw card
    glBindTexture(GL_TEXTURE_2D, texture);
    glBegin(GL_QUADS);
      glTexCoord2f(1.0f, 1.0f);
      glVertex3f(tr.x, tr.y, tr.z);
      glTexCoord2f(0.0f, 1.0f);
      glVertex3f(tl.x, tl.y, tl.z);
      glTexCoord2f(0.0f, 0.0f);
      glVertex3f(bl.x, bl.y, bl.z);
      glTexCoord2f(1.0f, 0.0f);
      glVertex3f(br.x, br.y, br.z);
    glEnd();
  }

} card;

void fullDeck()
{
  for(int i = 0; i < 52; i++)
  {
    deck[i] = true; // all cards available
  }
}

void initGL()
{
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClearDepth(1.0f);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  glEnable(GL_CULL_FACE);
  glEnable(GL_TEXTURE_2D);
  glShadeModel(GL_SMOOTH);
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
}

void
reshape(int w, int h)
{
  if(h == 0)
    h = 1;

  //winw = w;
  //winh = h;
  GLfloat aspect = (GLfloat)w / (GLfloat)h;
  glViewport(0, 0, w, h);       /* Establish viewing area to cover entire window. */
  glMatrixMode(GL_PROJECTION);  /* Start modifying the projection matrix. */
  glLoadIdentity();             /* Reset project matrix. */
  gluPerspective(45.0f, aspect, 0.1f, 100.0f);
}

void makeCards(int numCards)
{
  int index = rand() % 52;
  GLfloat offset = (360.0f / (GLfloat)52);
  GLfloat theta = 0.0f;

  for(int i = 0; i < numCards; i++)
  {
    //create a card
    while (!deck[index])
      index = rand() % 52;
    card c = card(index);

    c.getPoints(theta);

    cards.push_back(c);

    theta += offset;
  }
}

void nameCards()
{
  int i = 1;
  for(card n: cards)
  {
    std::cout<<"Card ID: "<<n.id
             <<"\n  Card Name: " <<pre2str(n.prefix)<<st2str(n.suit)
             <<"\n  Texture file path: "<<n.tfname
             <<"\n  Center Coordinates:"
             <<"\n    x: "<<n.center.x
             <<"\n    y: "<<n.center.y
             <<"\n    z: "<<n.center.z
             <<std::endl;
    i++;
  }
}

void drawCards()
{
  if(CC > -1)
    for(card n: cards)
    {
      glPushMatrix();
      n.render();
      glPopMatrix();
    }
}

void addCard()
{
  if (CC < 52)
  {
    int index = rand() % 52;
    GLfloat offset = 360.0f/52;

    GLfloat theta = 0.0;

    for(card n : cards)
    {
      theta += offset;
    }

    //create a card
    while (!deck[index])
      index = rand() % 52;
    card c = card(index);

    c.getPoints(theta);
    cards.push_back(c);
  }
}

void removeCard()
{
  if(CC > 0)
  {
    card c = cards.back();
    deck[c.index] = true;
    CC--;
    cards.pop_back();
  }
}

void display(void)
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glClearDepth(1.0f);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  gluLookAt(camx, camy, camz, //Position
            losx, losy, losz, //Line of sight
            upx, upy, upz);   //Up vector

  glTranslatef(0.0f, 0.0f, 0.0f);
  glRotatef(spin, 0.0, 0.5, 0.0);
  glTranslatef(0.0f, 0.0f, 0.0f);

  //draw cards
  drawCards();

  glutSwapBuffers();
}

void spinDisp(void) {
    spin = spin + (0.50 * sd);
    if(spin > 360.0)
      spin = spin - 360;
    if(spin < -360.0)
      spin = spin + 360;
    glutPostRedisplay();
}

void keys(unsigned char key, int x, int y)
{
  static GLfloat oldsd = 0;
  switch (key)
  {
    case 27:  //'esc'
      glutDestroyWindow(winID);
      exit(0);
      break;
    case 97:  //'a'
      addCard();
      break;
    case 114: //'r'
      if(CC > 0)
        removeCard();
      break;
    case 105: //'i'
      std::cout<<"\n#Cards: "<<CC<<"\n"<<std::endl;
      break;
    case 118: //'v'
      std::cout<<"\nVerbose Card Data:\n"<<std::endl;
      nameCards();
      break;
    case 32:  //'space'
      if(oldsd == 0)
      {
        oldsd = sd;
        sd = 0;
      }
      else
      {
        sd = oldsd;
        oldsd = 0;
      }
      break;
  }
  glutPostRedisplay();
}

void specKeys(int key, int x, int y)
{
  switch(key)
  {
    case GLUT_KEY_UP:
      camx = 0.0f; camy = 6.0f;  camz = 9.0f;
      losx = 0.0f; losy = -1.0f; losz = -1.0f;
      upx  = 0.0f; upy  = 1.0f;  upz  = -3.0f;
      break;
    case GLUT_KEY_DOWN:
      camx = 0.0f; camy = 0.0f; camz = 8.0f;
      losx = 0.0f; losy = 0.0f; losz = -1.0f;
      upx  = 0.0f; upy  = 1.0f; upz  = 0.0f;
      break;
  }
  glutPostRedisplay();
}

void mouse(int button, int state, int x, int y)
{
  switch(button)
  {
    case GLUT_LEFT_BUTTON:
      if(state == GLUT_DOWN)
      {
        sd = 1.0;
        glutIdleFunc(spinDisp);
      }
      break;
    case GLUT_RIGHT_BUTTON:
      if(state == GLUT_DOWN)
      {
        sd = -1.0;
        glutIdleFunc(spinDisp);
      }
      break;
  }
}

void initCam()
{
  //Initial View Parameters
  camx = 0.0f; camy = 6.0f; camz = 9.0f;
  losx = 0.0f; losy = -1.0f; losz = -1.0f;
  upx = 0.0f; upy = 1.0f; upz = -3.0f;
}

int main(int argc, char **argv)
{
  int NC;
  if(argc < 2)
      NC = 5;
  else
      NC = atoi(argv[1]);
  if(NC > 52 || NC <1)
  {
    std::cout<<"Invalid number of cards specified, try a value 0 < x < 53."<<std::endl;
    exit(0);
  }
  initCam();
  fullDeck();
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGB);
  glutInitWindowSize(640, 480);
  winID = glutCreateWindow("Card Carousel");
  makeCards(NC);
  //nameCards(); //sanity
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutKeyboardFunc(keys);
  glutSpecialFunc(specKeys);
  glutMouseFunc(mouse);
  initGL();
  glutMainLoop();
  return 0;
}
