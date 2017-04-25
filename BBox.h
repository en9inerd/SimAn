#ifndef _BBOX_H_
#define _BBOX_H_

#include "DataPlace.h"

class BBox {
   public:
    double xMin;
    double yMin;
    double xMax;
    double yMax;

    BBox(DataPlace& rbplace, bool forViewer = false);

    double getHeight() const { return fabs(yMax - yMin); }
    double getWidth() const { return fabs(xMax - xMin); }

   private:
    DataPlace& rb;
};

#endif