#include <ruby.h>
#include <ruby/encoding.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <opencv/cv.h>
#include <opencv/cxcore.h>
#include <opencv/highgui.h>

#define ID_CONST_GET rb_intern("const_get")
#define CONST_GET(scope, constant) (rb_funcall(scope, ID_CONST_GET, 1, rb_str_new2(constant)))

static VALUE rb_cFC;
static VALUE eLoadError;
static VALUE eRuntimeError;
static VALUE eArgumentError;

#define SILLY   1
#define EASY    2
#define MEDIUM  3
#define HARD    4

VALUE rb_captcha_image(VALUE self, VALUE string, VALUE lv) {
    CvScalar c;
    CvPoint *pts, pt;
    CvFont font[5];
    char text[2];
    IplImage *img;
    CvMat *out;
    int i, j, x, y;
    char *cstr = RSTRING_PTR(string);
    int W=strlen(cstr)*30+20, H=50;
    int level = NUM2INT(lv);
    double sx, sy;
    img = cvCreateImage(cvSize(W, H), 8, 3);
    cvSet(img, cvScalarAll(255), 0);
    pts = (CvPoint *)malloc(sizeof(CvPoint)*(strlen(cstr)+1));
    for (i = 0; i < 5; i++) {
        sx = (float)(rand()%4)*0.1 + 1.0;
        sy = (float)(rand()%4)*0.1 + 1.0;
        cvInitFont(&font[i], CV_FONT_HERSHEY_SIMPLEX, sx, sy, 1, 2, 16);
    }
    if (level > SILLY && level < HARD) {
        for (i = 0; i < W; i += 5) {
            c = CV_RGB(rand()%50 + 200, rand()%50 + 200, rand()%50 + 200);
            cvLine(img, cvPoint(i, 0), cvPoint(i, H), c, rand()%2+1, 8, 0);
        }
        for (i = 0; i < H; i += 5) {
            c = CV_RGB(rand()%50 + 200, rand()%50 + 200, rand()%50 + 200);
            cvLine(img, cvPoint(0, i), cvPoint(W, i), c, rand()%2+1, 8, 0);
        }
    }
    if (level >= MEDIUM) {
        for (i = 0; i < 10; i++) {
            j = rand()%50 + 120;
            pt = cvPoint(rand()%W, rand()%H);
            cvEllipse(img, pt, cvSize(rand()%50+10, rand()%50+10), rand()%90, 0, 360, CV_RGB(j,j,j), 1, 8, 0);
        }
    }
    x = rand()%40;
    memset(text, 0, 2);
    for (i = 0; i < strlen(cstr); i++) {
        y = (H>>1) - (10 - rand()%10) + 20;
        pts[i] = cvPoint(x, y - rand()%20);
        text[0] = cstr[i];
        c = CV_RGB(rand()%150 + 50, rand()%150 + 50, rand()%150 + 50);
        cvPutText(img, text, cvPoint(x, y), &font[i%5], c);
        x += 25;
        pts[i+1] = cvPoint(x, y - rand()%20);
    }
    if (level >= HARD) {
        for (i = 0; i < strlen(cstr); i++) {
            c = CV_RGB(rand()%50 + 120, rand()%50 + 120, rand()%50 + 120);
            cvLine(img, pts[i], pts[i+1], c, 2, 8, 0);
        }
    }
    out = cvEncodeImage(".png", img, 0);
    VALUE imgdata = rb_str_new((const char *)out->data.ptr, out->step);
    cvReleaseImage(&img);
    cvReleaseMat(&out);
    free(pts);
    return imgdata;
}

/* init */

void Init_fastcaptcha(void) {
    srand(time(NULL));
    eLoadError     = CONST_GET(rb_mKernel, "LoadError");
    eRuntimeError  = CONST_GET(rb_mKernel, "RuntimeError");
    eArgumentError = CONST_GET(rb_mKernel, "ArgumentError");
    rb_cFC = rb_define_class("FastCaptcha", rb_cObject);
    rb_define_method(rb_cFC, "image", rb_captcha_image, 2);
}
