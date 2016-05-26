#ifndef PERSISTENCE_H
#define PERSISTENCE_H

#include <array>
#include <qstring.h>

#include <topology/simplex.h>
#include <topology/filtration.h>


#include <QVector>
#include <QVector3D>

typedef std::array<double, 3> TPoint;

typedef std::vector<TPoint> PointList;

// Wrapper...
class Persistence
{
public:
    Persistence(): distance_(1) {}

    bool set_in_file(QString pts);
    void set_distance(double distance)            { distance_ = distance; }

    const QVector<QVector3D> get_points()         { return points_; }
    const PointList& get_chosen_pts()             { return pts_; }

    bool calculate();

private:
    QVector<QVector3D> calc_rips_();
    QVector<double> calcHomology(QVector<QVector3D> persistence, double dist);
    double datasetDistance();

    double distance_;
    PointList pts_;
    QVector<QVector3D> points_;
    Dimension skeleton = 3;
};

#endif // PERSISTENCE_H
