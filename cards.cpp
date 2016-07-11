#include <GL/glut.h>
#include <GL/freeglut.h>
#include <math.h>
#include <iostream>
#include <vector>
#include <string>
#include <boost/assign/list_of.hpp>
#include <boost/unordered_map.hpp>
#include <boost/format.hpp>

#include "SOIL.h"

#define PI 3.14159265

#define CW 1.5f
#define CH 2.0f
#define RC 4.0f

using boost::assign::map_list_of;
using boost::format;

struct card;

int winID;
int NC;
std::string tex_path = "./textures";
static GLfloat spin = 0.0;
static GLfloat sd = 1.0;
GLfloat camx, camy, camz, losx, losy, losz, upx, upy, upz;
bool deck[52];
std::vector<card> cards;

enum prefixes{ACE = 0,TWO = 1,THREE = 2,FOUR = 3,FIVE = 4,
              SIX = 5,SEVEN = 6,EIGHT = 7,NINE = 8,TEN = 9,
              JACK = 10,QUEEN = 11,KING = 12};
enum suits{DIAMONDS = 1,CLUBS = 2,HEARTS = 3,SPADES = 4};

const boost::unordered_map<prefixes,const char*> pre2str = map_list_of
  (ACE, "ace_of_")
  (TWO, "2_of_")
  (THREE, "3_of_")
  (FOUR, "4_of_")
  (FIVE, "5_of_")
  (SIX, "6_of_")
  (SEVEN, "7_of_")
  (EIGHT, "8_of_")
  (NINE, "9_of_")
  (TEN, "10_of_")
  (JACK, "jack_of_")
  (QUEEN, "queen_of_")
  (KING, "king_of_");

const boost::unordered_map<suits,const char*> st2str = map_list_of
  (DIAMONDS, "diamonds")
  (CLUBS, "clubs")
  (HEARTS, "hearts")
  (SPADES, "spades");

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
  std::string tfname;
  point tr,tl,bl,br,center;
  GLfloat theta;
  GLuint texture;

  card(int ind) : index (ind)
  {
    if(ind > 51 || ind < 0 || !deck[ind])
        return;

    int lb = 0;
    std::stringstream ss;

    tfname += tex_path;

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

    ss << format ("%s%s.png") %pre2str.at(prefix) %st2str.at(suit);
    tfname += '/';
    tfname += ss.str();

    //std::cout<<tfname<<std::endl;

    deck[ind] = false; // card taken, not available
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
  glEnable(GL_TEXTURE_2D);
  glShadeModel(GL_SMOOTH);
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
}

void
reshape(int w, int h)
{
  if(h == 0)
    h = 1;

  GLfloat aspect = (GLfloat)w / (GLfloat)h;
  glViewport(0, 0, w, h);       /* Establish viewing area to cover entire window. */
  glMatrixMode(GL_PROJECTION);  /* Start modifying the projection matrix. */
  glLoadIdentity();             /* Reset project matrix. */
  gluPerspective(45.0f, aspect, 0.1f, 100.0f);
}

void makeCards(int numCards)
{
  int index = rand() % 52;
  GLfloat offset = (360.0f / (GLfloat)numCards);
  GLfloat theta = 90.0;

  glColor3f(1.0f, 1.0f, 1.0f);
  for(int i = 0; i < numCards; i++)
  {
    //create a card
    while (!deck[index])
      index = rand() % 52;
    card c = card(index);

    //texture stuff
    c.texture = SOIL_load_OGL_texture
      (
        c.tfname.c_str(),
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_INVERT_Y
      );
    if(c.texture == 0)
        std::cout<<"SOIL ERROR"<<std::endl;

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
    std::cout<<"card "<<i<<" name: "<<n.tfname<<std::endl;
    i++;
  }
}

void drawCards()
{
  for(card n: cards)
  {
    glPushMatrix();
    //rotate cards
    glTranslatef(n.center.x, n.center.y, n.center.z);
    glRotatef(-spin, 0.0, 1.0, 0.0);
    glTranslatef(-1*n.center.x, -1*n.center.y, -1*n.center.z);
    //draw card
    glBindTexture(GL_TEXTURE_2D, n.texture);
    glBegin(GL_QUADS);
      glTexCoord2f(1.0f, 1.0f);
      glVertex3f(n.tr.x, n.tr.y, n.tr.z);
      glTexCoord2f(0.0f, 1.0f);
      glVertex3f(n.tl.x, n.tl.y, n.tl.z);
      glTexCoord2f(0.0f, 0.0f);
      glVertex3f(n.bl.x, n.bl.y, n.bl.z);
      glTexCoord2f(1.0f, 0.0f);
      glVertex3f(n.br.x, n.br.y, n.br.z);
    glEnd();
    glPopMatrix();
  }
}

void updateCards()
{
  GLfloat offset = 360.0/NC;
  for(card c : cards)
  {
    if(sd > 0)
      c.getPoints(c.theta+offset);
    else
      c.getPoints(c.theta-offset);
  }
}

void display(void)
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  gluLookAt(camx, camy, camz,  //Position
            losx, losy, losz, //Line of sight
            upx, upy, upz); //Up vector

  glTranslatef(0.0f, 0.0f, 0.0f);
  glRotatef(spin, 0.0, 0.5, 0.0);
  glTranslatef(0.0f, 0.0f, 0.0f);

  //draw cards
  drawCards();
  updateCards();

  glutSwapBuffers();
}

void spinDisp(void) {
    spin = spin + (2.0 * sd);
    if(spin > 360.0)
      spin = spin - 360;
    if(spin < -360.0)
      spin = spin + 360;
    glutPostRedisplay();
}

void keys(unsigned char key, int x, int y)
{
  switch (key)
  {
    case 27:
      glutDestroyWindow(winID);
      exit(0);
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
      else
        glutIdleFunc(NULL);
      break;
    case GLUT_RIGHT_BUTTON:
      if(state == GLUT_DOWN)
      {
        sd = -1.0;
        glutIdleFunc(spinDisp);
      }
      else
        glutIdleFunc(NULL);
      break;
  }
}

int
main(int argc, char **argv)
{
  if(argc < 2)
      NC = 26;
  else
      NC = atoi(argv[1]);
  if(NC > 52)
  {
    std::cout<<"Too many cards specified, try a value <= 52."<<std::endl;
    exit(0);
  }
  //Initial View Parameters
  camx = 0.0f; camy = 6.0f; camz = 9.0f;
  losx = 0.0f; losy = -1.0f; losz = -1.0f;
  upx = 0.0f; upy = 1.0f; upz = -3.0f;

  fullDeck();
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
  glutInitWindowSize(640, 480);
  winID = glutCreateWindow("card carousel");
  makeCards(NC);
  //nameCards(); //sanity
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutKeyboardFunc(keys);
  glutSpecialFunc(specKeys);
  glutMouseFunc(mouse);
  initGL();
  glutMainLoop();
  return 0;             /* ANSI C requires main to return int. */
}
