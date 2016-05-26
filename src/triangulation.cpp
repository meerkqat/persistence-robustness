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

#include "triangulation.h"

#define homology_append(frst, scnd, thrd) homology.append({static_cast<float>(frst), static_cast<float>(scnd), static_cast<float>(thrd)});

typedef         PairwiseDistances<PointContainer, L2Distance>           PairDistances;
typedef         PairDistances::DistanceType                             DistanceType;
typedef         PairDistances::IndexType                                Vertex;

typedef         Rips<PairDistances>                                     Generator;
typedef         Generator::Simplex                                      Smplx;
typedef         Filtration<Smplx>                                       Fltr;
typedef         DynamicPersistenceChains<>                              DynamicPersistence;
typedef         PersistenceDiagram<>                                    PDgm;

bool Triangulation::set_in_file(QString infile)
{
    orig_pts_.clear();
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
        if(!(line[0] == 'v' && line[1] == ' '))
            continue;

        QStringList  fields = line.right(line.length() - 2).split(QRegExp(",? "));

        double x = fields[0].toDouble();
        double y = fields[1].toDouble();
        double z = fields[2].toDouble();

        x_s += x; y_s += y; z_s += z;

        TPoint pt = {x, y, z};
        orig_pts_.push_back(pt);
    }

    x_s /= orig_pts_.size();
    y_s /= orig_pts_.size();
    z_s /= orig_pts_.size();

    for(uint i = 0; i < orig_pts_.size(); i++)
    {
        orig_pts_[i][0] -= x_s;
        orig_pts_[i][1] -= y_s;
        orig_pts_[i][2] -= z_s;
    }

    qDebug("Good file :)");
    return true;
}

bool Triangulation::calculate()
{
    std::random_shuffle(orig_pts_.begin(), orig_pts_.end());

    pts_ = PointList(orig_pts_.begin(), orig_pts_.begin() + int(orig_pts_.size()));

    triangles_.clear();
    lines_.clear();
    points_.clear();
    std::fill(homo_count_.begin(), homo_count_.end(), 0);
    done_ = false;

    calc_rips_();

    done_ = true;

    return true;
}

bool Triangulation::calc_rips_()
{
    DistanceType            max_distance;

    PointContainer          points;

    for(auto iter = pts_.begin(); iter != pts_.end(); iter++)
    {
        points.push_back(Point());

        TPoint tp = *iter;
        //x = tp.at(0); y = tp.at(1); z = tp.at(2);

        points.back().push_back(tp.at(0));
        points.back().push_back(tp.at(1));
        points.back().push_back(tp.at(2));
    }

    PairDistances           distances(points);
    Generator               rips(distances);
    Generator::Evaluator    size(distances);
    Fltr                    f;

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

    for(auto iter = p.begin(); iter != p.end(); iter++) {

        if(m[iter].dimension() == 0) continue;

        const std::vector<unsigned int> vtxs = m[iter].vertices();

        if(vtxs.size() == 1)
        {
            TPoint p = pts_[vtxs[0]];
            points_.push_back(QVector3D(p[0], p[1], p[2]));
        }
        else if(vtxs.size() == 2)
        {
            TPoint p1 = pts_[vtxs[0]];
            TPoint p2 = pts_[vtxs[1]];
            lines_.push_back(QVector3D(p1[0], p1[1], p1[2]));
            lines_.push_back(QVector3D(p2[0], p2[1], p2[2]));
        }
        else if(vtxs.size() == 3) {
            TTriangle to_add = { pts_[vtxs[0]], pts_[vtxs[1]], pts_[vtxs[2]] };
            add_triangle(to_add);
        }
        else if(vtxs.size() == 4) {
            TTriangle to_add1 = { pts_[vtxs[0]], pts_[vtxs[1]], pts_[vtxs[2]] };
            TTriangle to_add2 = { pts_[vtxs[1]], pts_[vtxs[2]], pts_[vtxs[3]] };
            TTriangle to_add3 = { pts_[vtxs[0]], pts_[vtxs[1]], pts_[vtxs[3]] };
            add_triangle(to_add1);
            add_triangle(to_add2);
            add_triangle(to_add3);
        }
    }


    for (DynamicPersistence::iterator cur = p.begin(); cur != p.end(); ++cur)
    {
        if (!cur->sign())        // only negative simplices have non-empty cycles
        {
            DynamicPersistence::OrderIndex birth = cur->pair;      // the cycle that cur killed was born when we added birth (another simplex)

            const Smplx& b = m[birth];
            const Smplx& d = m[cur];

            if (b.dimension() >= skeleton) continue;
            homology_append(b.dimension(), size(b), size(d));
        }
        else if (cur->unpaired())    // positive could be unpaired
        {
            const Smplx& b = m[cur];
            if (b.dimension() >= skeleton) continue;
            homology_append(b.dimension(), size(b), std::numeric_limits<float>::max());
        }
    }

    // calculate homology
    Q_FOREACH (auto h, homology) homo_count_[h[0]] += h[1] <= distance_ && distance_ <= h[2] ? 1 : 0;

    qDebug() << triangles_.size() << "triangles," << lines_.size() << "lines," << points_.size() << "points";
    qDebug() << "Vietoris-Rips finished!";

    return true;
}
