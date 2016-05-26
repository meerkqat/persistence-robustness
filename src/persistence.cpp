#include <topology/simplex.h>
#include <topology/rips.h>
#include <topology/filtration.h>
#include <topology/static-persistence.h>
#include <topology/dynamic-persistence.h>
#include <topology/persistence-diagram.h>

#include <geometry/l2distance.h>
#include <geometry/distances.h>

#include <utilities/types.h>

#include <QVector3D>
#include <vector>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/map.hpp>

#include <qdebug.h>
#include <qtextstream.h>
#include <qfile.h>

#include <sstream>
#include <iostream>
#include <fstream>

#include "persistence.h"

typedef         PairwiseDistances<PointContainer, L2Distance>           PairDistances;
typedef         PairDistances::DistanceType                             DistanceType;
typedef         PairDistances::IndexType                                Vertex;

typedef         Rips<PairDistances>                                     Generator;
typedef         Generator::Simplex                                      Smplx;
typedef         Filtration<Smplx>                                       Fltr;
typedef         DynamicPersistenceChains<>                              DynamicPersistence;
typedef         PersistenceDiagram<>                                    PDgm;

bool Persistence::set_in_file(QString infile)
{
    pts_.clear();
    QFile file(infile);

    if(!file.open(QIODevice::ReadOnly))
    {
        qDebug("Bad file!");
        return false;
    }

    double x_s=0, y_s=0, z_s=0;
    QTextStream in(&file);
    while(!in.atEnd())
    {
        QString line = in.readLine();
        // if line is comment or can't be at least "x y z"
        if(line[0] == '#' || line.size() < 5)
            continue;

        QStringList  fields = line.right(line.length() - 2).split(QRegExp(",? "));

        double x = fields[0].toDouble();
        double y = fields[1].toDouble();
        double z = fields[2].toDouble();

        x_s += x; y_s += y; z_s += z;

        TPoint pt = {x, y, z};
        pts_.push_back(pt);
    }

    x_s /= pts_.size();
    y_s /= pts_.size();
    z_s /= pts_.size();

    for(uint i = 0; i < pts_.size(); i++)
    {
        pts_[i][0] -= x_s;
        pts_[i][1] -= y_s;
        pts_[i][2] -= z_s;
    }

    qDebug("Good file :)");
    return true;
}

bool Persistence::calculate()
{
    points_.clear();

    QVector<QVector3D> persistence = calc_rips_();

    double max_dist = datasetDistance();

    for (int i = 0; i < 11; i++) {
        qDebug() << "Distance " << (max_dist/10.0)*i;
        qDebug() << calcHomology(persistence, (max_dist/10.0)*i);
    }

    return true;
}

double Persistence::datasetDistance()
{
    // TODO return max distance between 2 points in dataset
    return 1.0;
}

QVector<QVector3D> Persistence::calc_rips_()
{
    DistanceType max_distance;

    PointContainer points;

    for(auto iter = pts_.begin(); iter != pts_.end(); iter++)
    {
        points.push_back(Point());

        TPoint tp = *iter;
        //x = tp.at(0); y = tp.at(1); z = tp.at(2);

        points.back().push_back(tp.at(0));
        points.back().push_back(tp.at(1));
        points.back().push_back(tp.at(2));
    }

    PairDistances distances(points);
    Generator rips(distances);
    Generator::Evaluator size(distances);
    Fltr f;

    max_distance = distance_;

    // Generate n-skeleton of the Rips complex
    rips.generate(skeleton, max_distance, make_push_back_functor(f));
    std::cout << "# Generated complex of size: " << f.size() << std::endl;

    // Generate filtration with respect to distance and compute its persistence
    f.sort(Generator::Comparison(distances));

    DynamicPersistence p(f);
    p.pair_simplices();

    // Output cycles
    DynamicPersistence::SimplexMap<Fltr> m = p.make_simplex_map(f);

    QVector<QVector3D> persistence;
    for (DynamicPersistence::iterator cur = p.begin(); cur != p.end(); ++cur) {
        // only negative simplices have non-empty cycles
        if (!cur->sign()) {
            // the cycle that cur killed was born when we added birth (another simplex)
            DynamicPersistence::OrderIndex birth = cur->pair;

            const Smplx& b = m[birth];
            const Smplx& d = m[cur];

            if (b.dimension() >= skeleton) continue;
            persistence.append({static_cast<float>(b.dimension()), static_cast<float>(size(b)), static_cast<float>(size(d))});
        }
        // positive could be unpaired
        else if (cur->unpaired()) {
            const Smplx& b = m[cur];
            if (b.dimension() >= skeleton) continue;
            persistence.append({static_cast<float>(b.dimension()), static_cast<float>(size(b)), std::numeric_limits<float>::max()});
        }
    }

    qDebug() << points_.size() << "points";
    qDebug() << "Vietoris-Rips finished!";

    return persistence;
}

QVector<double> Persistence::calcHomology(QVector<QVector3D> persistence, double dist)
{
    QVector<double> homo_count(skeleton,0);
    Q_FOREACH (auto h, persistence) homo_count[h[0]] += h[1] <= dist && dist <= h[2] ? 1 : 0;

    return homo_count;
}
