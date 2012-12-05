// This file is part of snark, a generic and flexible library
// for robotics research.
//
// Copyright (C) 2011 The University of Sydney
//
// snark is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or (at your option) any later version.
//
// snark is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License
// for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with snark. If not, see <http://www.gnu.org/licenses/>.

/// @author Vsevolod Vlaskine, Cedric Wohlleber

#ifndef SNARK_GRAPHICS_APPLICATIONS_VIEWPOINTS_SHAPEWITHID_H_
#define SNARK_GRAPHICS_APPLICATIONS_VIEWPOINTS_SHAPEWITHID_H_

#include <boost/foreach.hpp>
#include <boost/optional.hpp>
#include <comma/base/types.h>
#include <comma/visiting/traits.h>
#include <snark/math/interval.h>
#include <snark/graphics/qt3d/rotation_matrix.h>
#include <snark/graphics/qt3d/vertex_buffer.h>
#include <Qt3D/qglnamespace.h>
#include <Qt3D/qglpainter.h>

namespace snark { namespace graphics { namespace View {

template < class S >
struct ShapeWithId // quick and dirty
{
    typedef S Shape;
    ShapeWithId() : id( 0 ), block( 0 ) {}
    S shape;
    comma::uint32 id;
    comma::uint32 block;
    QColor4ub color;
    std::string label;
    double scalar;
};


template < class S >
struct Shapetraits {}; // quick and dirty

template<>
struct Shapetraits< Eigen::Vector3d >
{
    static const QGL::DrawingMode drawingMode = QGL::Points;
    static const unsigned int size = 1;
    
    static void update( const Eigen::Vector3d& p, const Eigen::Vector3d& offset, const QColor4ub& color, unsigned int block, qt3d::vertex_buffer& buffer, boost::optional< snark::math::closed_interval< float, 3 > >& extents  )
    {
        Eigen::Vector3d point = p - offset;
        buffer.addVertex( QVector3D( point.x(), point.y(), point.z() ), color, block );
        if( extents )
        {
            extents = extents->hull( point.cast< float >() );
        }
        else
        {
            extents = snark::math::closed_interval< float, 3 >( point.cast< float >() );
        }
    }

    static void draw( QGLPainter* painter, unsigned int size, unsigned int index )
    {
        painter->draw( QGL::Points, size, index );
    }
    
    static const Eigen::Vector3d& somePoint( const Eigen::Vector3d& point ) { return point; }
    
    static const Eigen::Vector3d& center( const Eigen::Vector3d& point ) { return point; }
};

template<>
struct Shapetraits< snark::math::closed_interval< double, 3 > >
{
    static const unsigned int size = 8;
    static void update( const snark::math::closed_interval< double, 3 >& e, const Eigen::Vector3d& offset, const QColor4ub& color, unsigned int block, qt3d::vertex_buffer& buffer, boost::optional< snark::math::closed_interval< float, 3 > >& extents  )
    {
        Eigen::Vector3f min = ( e.min() - offset ).cast< float >();
        Eigen::Vector3f max = ( e.max() - offset ).cast< float >();
        buffer.addVertex( QVector3D( min.x(), min.y(), min.z() ), color, block );
        buffer.addVertex( QVector3D( min.x(), min.y(), max.z() ), color, block );
        buffer.addVertex( QVector3D( min.x(), max.y(), max.z() ), color, block );
        buffer.addVertex( QVector3D( min.x(), max.y(), min.z() ), color, block );
        buffer.addVertex( QVector3D( max.x(), min.y(), min.z() ), color, block );
        buffer.addVertex( QVector3D( max.x(), min.y(), max.z() ), color, block );
        buffer.addVertex( QVector3D( max.x(), max.y(), max.z() ), color, block );
        buffer.addVertex( QVector3D( max.x(), max.y(), min.z() ), color, block );
        
        if( extents )
        {
            extents = extents->hull( snark::math::closed_interval< float, 3 >( min, max ) );
        }
        else
        {
            extents = snark::math::closed_interval< float, 3 >( min, max );
        }
    }

    static void draw( QGLPainter* painter, unsigned int size, unsigned int index )
    {
        const boost::array< unsigned short, 8  > baseIndices = { { 0, 4, 1, 5, 2, 6, 3, 7 } };
        for( unsigned int i = 0; i < size; i += 8 )
        {
            // 2 line loops
            painter->draw( QGL::LineLoop, 4, index + i );
            painter->draw( QGL::LineLoop, 4, index + i + 4 );

            // 4 lines
            boost::array< unsigned short, 8  > lineIndices = baseIndices;
            BOOST_FOREACH( unsigned short& j, lineIndices )
            {
                j += index + i;
            }
            painter->draw( QGL::Lines, &lineIndices[0], 8 );
        }
    }
    
    static const Eigen::Vector3d& somePoint( const snark::math::closed_interval< double, 3 >& extents ) { return extents.min(); }
    
    static Eigen::Vector3d center( const snark::math::closed_interval< double, 3 >& extents ) { return ( extents.min() + extents.max() ) / 2; }
};

template<>
struct Shapetraits< std::pair< Eigen::Vector3d, Eigen::Vector3d > >
{
    static const unsigned int size = 2;
    static void update( const std::pair< Eigen::Vector3d, Eigen::Vector3d >& p, const Eigen::Vector3d& offset, const QColor4ub& color, unsigned int block, qt3d::vertex_buffer& buffer, boost::optional< snark::math::closed_interval< float, 3 > >& extents  )
    {
        Eigen::Vector3f first = ( p.first - offset ).cast< float >();
        Eigen::Vector3f second = ( p.second - offset ).cast< float >();
        buffer.addVertex( QVector3D( first.x(), first.y(), first.z() ), color, block );
        buffer.addVertex( QVector3D( second.x(), second.y(), second.z() ), color, block );
        if( extents )
        {
            extents = extents->hull( snark::math::closed_interval< float, 3 >( first, second ) );
        }
        else
        {
            extents = snark::math::closed_interval< float, 3 >( first, second );
        }
    }

    static void draw( QGLPainter* painter, unsigned int size, unsigned int index )
    {
        painter->draw( QGL::Lines, size, index );
    }
    
    static const Eigen::Vector3d& somePoint( const std::pair< Eigen::Vector3d, Eigen::Vector3d >& line ) { return line.first; }
    
    static Eigen::Vector3d center( const std::pair< Eigen::Vector3d, Eigen::Vector3d >& line ) { return ( line.first + line.second ) / 2; }
};

template < std::size_t Size >
struct Ellipse
{
    Eigen::Vector3d center;
    Eigen::Vector3d orientation;
    double major;
    double minor;
};

template < std::size_t Size >
struct Shapetraits< Ellipse< Size > >
{
    static const unsigned int size = Size;
    static void update( const Ellipse< Size >& ellipse, const Eigen::Vector3d& offset, const QColor4ub& color, unsigned int block, qt3d::vertex_buffer& buffer, boost::optional< snark::math::closed_interval< float, 3 > >& extents  )
    {
        Eigen::Vector3d c = ellipse.center - offset;
        const Eigen::Matrix3d& r = rotation_matrix::rotation( ellipse.orientation );
        static const double step = 3.14159265358979323846l * 2 / Size;
        double angle = 0;
        // todo: use native opengl rotation and normals instead
        for( std::size_t i = 0; i < Size; ++i, angle += step )
        {
            Eigen::Vector3d v = r * Eigen::Vector3d( std::cos( angle ) * ellipse.major, std::sin( angle ) * ellipse.minor, 0 );
            Eigen::Vector3d p( v.x(), v.y(), v.z() );
            Eigen::Vector3f point = ( p + c ).cast< float >();
            buffer.addVertex( QVector3D( point.x(), point.y(), point.z() ), color, block );
            if( extents )
            {
                extents = extents->hull( point );
            }
            else
            {
                extents = snark::math::closed_interval< float, 3 >( point );
            }
        }
    }

    static void draw( QGLPainter* painter, unsigned int size, unsigned int index )
    {
        for( unsigned int i = 0; i < size; i += Size )
        {
            painter->draw( QGL::LineLoop, Size, index + i );
        }
    }
    
    static const Eigen::Vector3d& somePoint( const Ellipse< Size >& ellipse ) { return ellipse.center; }

    static Eigen::Vector3d center( const Ellipse< Size >& ellipse ) { return ellipse.center; }
};

} } }

namespace comma { namespace visiting {

template <> struct traits< QColor4ub >
{
    template < typename Key, class Visitor >
    static void visit( Key, QColor4ub& p, Visitor& v )
    {
        unsigned char red = 0;
        unsigned char green = 0;
        unsigned char blue = 0;
        unsigned char alpha = 255;
        v.apply( "r", red );
        v.apply( "g", green );
        v.apply( "b", blue );
        v.apply( "a", alpha );
        p = QColor4ub( red, green, blue, alpha );
    }

    template < typename Key, class Visitor >
    static void visit( Key, const QColor4ub& p, Visitor& v )
    {
        v.apply( "r", p.red() );
        v.apply( "g", p.green() );
        v.apply( "b", p.blue() );
        v.apply( "a", p.alpha() );
    }
};
    
template < typename S > struct traits< snark::graphics::View::ShapeWithId< S > >
{
    template < typename Key, class Visitor >
    static void visit( Key, snark::graphics::View::ShapeWithId< S >& p, Visitor& v )
    {
        v.apply( "shape", p.shape );
        v.apply( "id", p.id );
        v.apply( "block", p.block );
        v.apply( "colour", p.color );
        v.apply( "label", p.label );
        v.apply( "scalar", p.scalar );
    }

    template < typename Key, class Visitor >
    static void visit( Key, const snark::graphics::View::ShapeWithId< S >& p, Visitor& v )
    {
        v.apply( "shape", p.shape );
        v.apply( "id", p.id );
        v.apply( "block", p.block );
        v.apply( "colour", p.color );
        v.apply( "label", p.label );
        v.apply( "scalar", p.scalar );
    }
};

template < std::size_t Size > struct traits< snark::graphics::View::Ellipse< Size > >
{
    template < typename Key, class Visitor >
    static void visit( Key, snark::graphics::View::Ellipse< Size >& p, Visitor& v )
    {
        v.apply( "center", p.center );
        v.apply( "orientation", p.orientation );
        v.apply( "major", p.major );
        v.apply( "minor", p.minor );
    }

    template < typename Key, class Visitor >
    static void visit( Key, const snark::graphics::View::Ellipse< Size >& p, Visitor& v )
    {
        v.apply( "center", p.center );
        v.apply( "orientation", p.orientation );
        v.apply( "major", p.major );
        v.apply( "minor", p.minor );
    }
};

template < typename T > struct traits< snark::math::closed_interval< T, 3 > >
{
    template < typename Key, class Visitor >
    static void visit( Key, snark::math::closed_interval< T, 3 >& p, Visitor& v )
    {
        Eigen::Matrix< T, 3, 1 > min;
        Eigen::Matrix< T, 3, 1 > max;
        v.apply( "min", min );
        v.apply( "max", max );
        p = snark::math::closed_interval< T, 3 >( min, max );
    }

    template < typename Key, class Visitor >
    static void visit( Key, const snark::math::closed_interval< T, 3 >& p, Visitor& v )
    {
        v.apply( "min", p.min() );
        v.apply( "max", p.max() );
    }
};

} } // namespace comma { namespace visiting {

#endif /*SNARK_GRAPHICS_APPLICATIONS_VIEWPOINTS_SHAPEWITHID_H_*/