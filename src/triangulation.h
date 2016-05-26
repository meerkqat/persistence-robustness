#ifndef TRIANGULATION_H
#define TRIANGULATION_H

#include <array>
#include <qstring.h>

#include <topology/simplex.h>
#include <topology/filtration.h>


#include <QVector>
#include <QVector3D>

typedef std::array<double, 3> TPoint;
typedef std::array<TPoint, 3> TTriangle;

typedef std::vector<TPoint> PointList;
typedef std::vector<TTriangle> TriangleList;

// Wrapper...
class Triangulation
{
public:
    Triangulation(): done_(false), distance_(1), homo_count_(skeleton, 0) {}

    bool set_in_file(QString pts);
    void set_distance(double distance)            { distance_ = distance; }

    const TriangleList& get_triangles()           { return triangles_; }
    const QVector<QVector3D> get_lines()          { return lines_; }
    const QVector<QVector3D> get_points()         { return points_; }
    const PointList& get_chosen_pts()             { return pts_; }
    const std::vector<double>& get_homology()     { return homo_count_; }

    bool calculate();

    void add_triangle(TTriangle t) { triangles_.push_back(t); }

private:
    bool calc_rips_();

    QVector<QVector3D> homology;
    bool done_;
    double distance_;
    PointList pts_;
    PointList orig_pts_;
    TriangleList triangles_;
    QVector<QVector3D> lines_;
    QVector<QVector3D> points_;
    Dimension skeleton = 3;
    std::vector<double> homo_count_;
};

#endif // TRIANGULATION_H
