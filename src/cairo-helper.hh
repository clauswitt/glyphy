/*
 * Copyright 2012 Google, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Google Author(s): Behdad Esfahbod, Maysum Panju
 */

#include <cairo.h>

#include <stdio.h>
#include <algorithm>
#include <vector>
#include <queue>

#include <glyphy/geometry.hh>

#ifndef CAIRO_HELPER_HH
#define CAIRO_HELPER_HH

namespace GLyphy {

namespace CairoHelper {

using namespace Geometry;

template <typename Coord>
void cairo_move_to (cairo_t *cr, const Point<Coord> &p)
{
  cairo_move_to (cr, p.x, p.y);
}

template <typename Coord>
void cairo_line_to (cairo_t *cr, const Point<Coord> &p)
{
  cairo_line_to (cr, p.x, p.y);
}

template <typename Coord>
void cairo_rel_move_to (cairo_t *cr, const Vector<Coord> &v)
{
  cairo_rel_move_to (cr, v.dx, v.dy);
}

template <typename Coord>
void cairo_rel_line_to (cairo_t *cr, const Vector<Coord> &v)
{
  cairo_rel_line_to (cr, v.dx, v.dy);
}


template <typename Coord>
void cairo_point (cairo_t *cr, const Point<Coord> &p)
{
  cairo_move_to (cr, p.x, p.y);
  cairo_rel_line_to (cr, 0, 0);
}

template <typename Coord>
void cairo_line (cairo_t *cr, const Line<Coord> &l)
{
  cairo_new_sub_path (cr);
#define BIG 10000
  if (!l.a)
    for (unsigned int i = -BIG; i <= BIG; i += 2*BIG)
      cairo_line_to (cr, i, l.c / l.b);
  else if (!l.b)
    for (unsigned int i = -BIG; i <= BIG; i += 2*BIG)
      cairo_line_to (cr, l.c / l.a, i);
  else
    for (unsigned int i = -BIG; i <= BIG; i += 2*BIG)
      cairo_line_to (cr, i, (l.c - l.a * i) / l.b);
#undef BIG
}

template <typename Coord, typename Scalar>
void cairo_circle (cairo_t *cr, const Circle<Coord, Scalar> &c)
{
  cairo_new_sub_path (cr);
  cairo_arc (cr, c.c.x, c.c.y, 0, 2 * M_PI);
}

template <typename Coord, typename Scalar>
void cairo_arc (cairo_t *cr, const Arc<Coord, Scalar> &a)
{
  if (fabs (a.d) < 1e-6) {
    cairo_line_to (cr, a.p0);
    cairo_line_to (cr, a.p1);
    return;
  }

  Circle<Coord, Scalar> c  = a.circle ();
  double a0 = (a.p0 - c.c).angle ();
  double a1 = (a.p1 - c.c).angle ();
  if (a.d < 0)
    cairo_arc (cr, c.c.x, c.c.y, c.r, a0, a1);
  else
    cairo_arc_negative (cr, c.c.x, c.c.y, c.r, a0, a1);
}

template <typename Coord, typename Scalar>
void cairo_arcs (cairo_t *cr, const std::vector<Arc<Coord, Scalar> > &arcs)
{
  Point<Coord> start (0, 0);
  for (unsigned int i = 0; i < arcs.size (); i++) {
    if (!cairo_has_current_point (cr))
      start = arcs[i].p0;
    cairo_arc (cr, arcs[i]);
    if (arcs[i].p1 == start) {
      cairo_close_path (cr);
      cairo_new_sub_path (cr);
    }
  }
}

template <typename Coord>
void cairo_curve (cairo_t *cr, const Bezier<Coord> &b)
{
  cairo_line_to (cr, b.p0.x, b.p0.y);
  cairo_curve_to (cr,
		  b.p1.x, b.p1.y,
		  b.p2.x, b.p2.y,
		  b.p3.x, b.p3.y);
}



template <typename Coord>
void cairo_demo_point (cairo_t *cr, const Point<Coord> &p)
{
  cairo_save (cr);
  cairo_set_line_cap (cr, CAIRO_LINE_CAP_ROUND);
  cairo_move_to (cr, p);
  cairo_rel_line_to (cr, 0, 0);
  cairo_set_line_width (cr, cairo_get_line_width (cr) * 3);
  cairo_stroke (cr);
  cairo_restore (cr);
}

template <typename Coord>
void cairo_demo_curve (cairo_t *cr, const Bezier<Coord> &b)
{

  /* Draw the sampled lines in the sector of the circle. 
  {
    cairo_save (cr);
    for (double t = 0; t <= 1; t += .01)
    {
      Point<Coord> p = b.point (t);
      Circle<Coord, Scalar> cv = b.osculating_circle (t);
      cairo_move_to (cr, p.x, p.y);
      cairo_line_to (cr, cv.c.x, cv.c.y);
    }
    cairo_set_line_width (cr, cairo_get_line_width (cr) / 30);
    cairo_stroke (cr);
    cairo_restore (cr);
  }*/

  /* Draw the points determining the curve. */
  cairo_demo_point (cr, b.p0);
 /* cairo_demo_point (cr, b.p1);
  cairo_demo_point (cr, b.p2);*/
  cairo_demo_point (cr, b.p3); 

  /* Draw the lines connecting p0/p1, p1/p2.
  cairo_save (cr);
  cairo_move_to (cr, b.p0);
  cairo_line_to (cr, b.p1);
  cairo_move_to (cr, b.p2);
  cairo_line_to (cr, b.p3);
  cairo_set_line_width (cr, cairo_get_line_width (cr) / 3);
  cairo_stroke (cr);
  cairo_restore (cr); */

  /* Draw the actual curve. */
  cairo_curve (cr, b);
  cairo_stroke (cr);
}

template <typename Coord>
void cairo_demo_arc (cairo_t *cr, const Arc<Coord, Scalar> &a)
{

 // printf("cairo_demo_arc. p0: (%g, %g). p1: (%g, %g). d: %f.\n", a.p0.x, a.p0.y, a.p1.x, a.p1.y, a.d);

  if (fabs (a.d) < 1e-6) {
    cairo_move_to (cr, a.p0);
    cairo_line_to (cr, a.p1);
    cairo_stroke (cr);
    
    cairo_set_line_width (cr, cairo_get_line_width (cr) / 2);
    cairo_demo_point (cr, a.p0);
    cairo_demo_point (cr, a.p1);
    cairo_set_line_width (cr, cairo_get_line_width (cr) * 2);
    
    return;
  }
 /*  
   //  Draw a distance field around the arc..
   for (double x = -10; x < 30; x += 0.02) {
      for (double y = -10; y < 20; y += 0.02) {
        Point<Coord> p (x, y);
        double z = (a - p).len ();
        double y = (a - p).negative ? 0.2 : 1;
        cairo_set_source_rgb (cr, y, 1.1 * z, 1.2 * z);
        cairo_demo_point (cr, p);
      }
    }  */




  Circle<Coord, Scalar> c  = a.circle ();
  double a0 = (a.p0 - c.c).angle ();
  double a1 = (a.p1 - c.c).angle ();

  cairo_save (cr);
 /* cairo_move_to (cr, a.p1);
  cairo_line_to (cr, c.c);
  cairo_line_to (cr, a.p0);
  if (a.d < 0)
    cairo_arc (cr, c.c.x, c.c.y, c.r, a0, a1);
  else
    cairo_arc_negative (cr, c.c.x, c.c.y, c.r, a0, a1);
  cairo_close_path (cr);
  cairo_clip (cr);
  cairo_paint_with_alpha (cr, .2);
  cairo_restore (cr);
*/

  cairo_set_line_width (cr, cairo_get_line_width (cr) / 3);
  cairo_demo_point (cr, a.p0);
  cairo_demo_point (cr, a.p1);
  cairo_set_line_width (cr, cairo_get_line_width (cr) * 9);
  
/*
  cairo_save (cr);
  cairo_move_to (cr, a.p0);
  cairo_line_to (cr, c.c);
  cairo_line_to (cr, a.p1);
  cairo_set_line_width (cr, cairo_get_line_width (cr) / 3);
  cairo_stroke (cr);
  cairo_restore (cr);

  cairo_new_sub_path (cr);*/
  if (a.d < 0)
    cairo_arc (cr, c.c.x, c.c.y, c.r, a0, a1);
  else
    cairo_arc_negative (cr, c.c.x, c.c.y, c.r, a0, a1);

  cairo_stroke (cr); 
  
//  cairo_set_source_rgb (0, 0, 0);
  
  

  
}

template <typename Coord>
void cairo_demo_arcs (cairo_t *cr, const std::vector<Arc<Coord, Scalar> > &arcs)
{
  for (unsigned int i = 0; i < arcs.size (); i++)
    cairo_demo_arc (cr, arcs[i]);
}

/* A fancy cairo_stroke_preserve() that draws points and control
 * points, and connects them together.
 */
static void
cairo_fancy_stroke_preserve (cairo_t *cr)
{
  int i;
  double line_width;
  cairo_path_t *path;
  cairo_path_data_t *data;

  cairo_save (cr);

  line_width = cairo_get_line_width (cr);
  path = cairo_copy_path (cr);
  cairo_new_path (cr);

  cairo_save (cr);
  cairo_set_line_width (cr, line_width / 3);
  for (i=0; i < path->num_data; i += path->data[i].header.length) {
    data = &path->data[i];
    switch (data->header.type) {
    case CAIRO_PATH_MOVE_TO:
    case CAIRO_PATH_LINE_TO:
	cairo_move_to (cr, data[1].point.x, data[1].point.y);
	break;
    case CAIRO_PATH_CURVE_TO:
	cairo_line_to (cr, data[1].point.x, data[1].point.y);
	cairo_move_to (cr, data[2].point.x, data[2].point.y);
	cairo_line_to (cr, data[3].point.x, data[3].point.y);
	break;
    case CAIRO_PATH_CLOSE_PATH:
	break;
    default:
	break;
    }
  }
  cairo_stroke (cr);
  cairo_restore (cr);

  cairo_save (cr);
  cairo_set_line_width (cr, line_width * 2);
  cairo_set_line_cap (cr, CAIRO_LINE_CAP_ROUND);
  for (i=0; i < path->num_data; i += path->data[i].header.length) {
    data = &path->data[i];
    switch (data->header.type) {
    case CAIRO_PATH_MOVE_TO:
	cairo_move_to (cr, data[1].point.x, data[1].point.y);
	break;
    case CAIRO_PATH_LINE_TO:
	cairo_rel_line_to (cr, 0, 0);
	cairo_move_to (cr, data[1].point.x, data[1].point.y);
	break;
    case CAIRO_PATH_CURVE_TO:
	cairo_rel_line_to (cr, 0, 0);
	cairo_move_to (cr, data[1].point.x, data[1].point.y);
	cairo_rel_line_to (cr, 0, 0);
	cairo_move_to (cr, data[2].point.x, data[2].point.y);
	cairo_rel_line_to (cr, 0, 0);
	cairo_move_to (cr, data[3].point.x, data[3].point.y);
	break;
    case CAIRO_PATH_CLOSE_PATH:
	cairo_rel_line_to (cr, 0, 0);
	break;
    default:
	break;
    }
  }
  cairo_rel_line_to (cr, 0, 0);
  cairo_stroke (cr);
  cairo_restore (cr);

  cairo_append_path (cr, path);
  cairo_path_destroy (path);

  cairo_stroke_preserve (cr);

  cairo_restore (cr);
}

/* A fancy cairo_stroke() that draws points and control points, and
 * connects them together.
 */
void
cairo_fancy_stroke (cairo_t *cr)
{
  cairo_fancy_stroke_preserve (cr);
  cairo_new_path (cr);
}

static void
cairo_path_print_stats (const cairo_path_t *path)
{
  int i;
  cairo_path_data_t *data;
  int lines = 0, curves = 0;

  for (i=0; i < path->num_data; i += path->data[i].header.length) {
    data = &path->data[i];
    switch ((int) data->header.type) {
    case CAIRO_PATH_MOVE_TO:
        break;
    case CAIRO_PATH_LINE_TO:
	lines++;
	break;
    case CAIRO_PATH_CURVE_TO:
	curves++;
	break;
    case CAIRO_PATH_CLOSE_PATH:
	break;
    default:
	break;
    }
  }
  printf ("%d pieces = %d lines and %d curves\n", lines + curves, lines, curves);
}

static void
cairo_set_viewport (cairo_t *cr)
{
  double cx1, cy1, cx2, cy2;
  cairo_clip_extents (cr, &cx1, &cy1, &cx2, &cy2);

  double px1, py1, px2, py2;
  cairo_path_extents (cr, &px1, &py1, &px2, &py2);

  double scale = .8 / std::max ((px2 - px1) / (cx2 - cx1), (py2 - py1) / (cy2 - cy1));

  cairo_translate (cr, (cx1 + cx2) * .5, (cy1 + cy2) * .5);
  cairo_scale (cr, scale, /*-*/scale);
  cairo_set_line_width (cr, cairo_get_line_width (cr) / scale);

  cairo_translate (cr, -(px1 + px2) * .5, -(py1 + py2) * .5);
}


class CairoOutlineSink
{
  public:

  CairoOutlineSink (cairo_t *cr_)
  : cr (cairo_reference (cr_)) {}

  cairo_t *cr;

  bool
  move_to (const Point<Coord> &p)
  {
    cairo_close_path (cr);
    cairo_move_to (cr, p);
    return true;
  }

  bool
  line_to (const Point<Coord> &p1)
  {
    cairo_line_to (cr, p1);
    return true;
  }

  bool
  conic_to (const Point<Coord> &p1, const Point<Coord> &p2)
  {
    double x, y;
    cairo_get_current_point (cr, &x, &y);
    Point<Coord> p0 (x, y);
    return cubic_to (p0 + 2/3. * (p1 - p0),
		     p2 + 2/3. * (p1 - p2),
		     p2);
  }

  bool
  cubic_to (const Point<Coord> &p1, const Point<Coord> &p2, const Point<Coord> &p3)
  {
    cairo_curve_to (cr, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y);
    return true;
  }

  bool
  arc (const Arc<Coord, Scalar> &a)
  {
    cairo_arc (cr, a);
    return true;
  }

  bool
  bezier (const Bezier<Coord> &b)
  {
    cairo_curve (cr, b);
    return true;
  }
};


} /* namespace CairoHelper */

} /* namespace GLyphy */
#endif