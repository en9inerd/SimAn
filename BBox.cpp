#include "BBox.h"
#include <tuple>

BBox::BBox(DataPlace& rbplace, bool forViewer)
    : rb(rbplace),
      xMin(DBL_MAX),
      yMin(DBL_MAX),
      xMax(-DBL_MAX),
      yMax(-DBL_MAX) {
    vector<row>& _rw = rb.rows;

    vector<terminal>::iterator termMinY, termMaxY;
    vector<terminal>::iterator termMinX, termMaxX;
    if (!rb.terminals.empty()) {
        tie(termMinY, termMaxY) =
            minmax_element(rb.terminals.begin(), rb.terminals.end(),
                           [](terminal const& s1, terminal const& s2) {
                               return s1.pos_y < s2.pos_y;
                           });

        tie(termMinX, termMaxX) =
            minmax_element(rb.terminals.begin(), rb.terminals.end(),
                           [](terminal const& s1, terminal const& s2) {
                               return s1.pos_x < s2.pos_x;
                           });
    }

    double CoreXmin = _rw.begin()->coord_x;
    double CoreYmin = _rw.begin()->coord_y;
    double CoreXmax =
        CoreXmin + (_rw.begin()->num_sites) * (_rw.begin()->site_sp);
    double CoreYmax = CoreYmin + rb.NumRows * _rw.begin()->h;

    if (!rb.terminals.empty() && forViewer) {
        yMin = (termMinY->pos_y > CoreYmin) ? CoreYmin - _rw.begin()->site_sp
                                            : termMinY->pos_y;
        yMax = (termMaxY->pos_y > CoreYmax) ? termMaxY->pos_y + termMaxX->h
                                            : CoreYmax + _rw.begin()->site_sp;
        xMin = (termMinX->pos_x > CoreXmin) ? CoreXmin - _rw.begin()->site_sp
                                            : termMinX->pos_x;
        xMax = (termMaxX->pos_x > CoreXmax) ? termMaxX->pos_x + termMaxX->w
                                            : CoreXmax + _rw.begin()->site_sp;
    } else if (rb.terminals.empty() && forViewer) {
        yMin = CoreYmin - _rw.begin()->site_sp;
        xMin = CoreXmin - _rw.begin()->site_sp;
        yMax = CoreYmax + _rw.begin()->site_sp;
        xMax = CoreXmax + _rw.begin()->site_sp;
    } else {
        yMin = CoreYmin;
        xMin = CoreXmin;
        yMax = CoreYmax;
        xMax = CoreXmax;
    }
}