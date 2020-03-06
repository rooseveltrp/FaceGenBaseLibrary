//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#ifndef FGAPPROXEQUAL_HPP
#define FGAPPROXEQUAL_HPP

#include "FgTypes.hpp"
#include "FgMath.hpp"
#include "FgMatrixC.hpp"
#include "FgMatrixV.hpp"

namespace Fg {

// Convert from bits of precision to an epsilon value for that precision:
inline double bitsToPrecision(uint bits) {return 1.0 / double(1ULL << bits); }
inline float  bitsToPrecisionF(uint bits) {return 1.0f / float(1ULL << bits); }

// Are two numbers approximately equal relative to an absolute scale ?
template<typename T>
inline bool
isApproxEqual(T v0,T v1,T maxDiff)
{return (std::abs(v1-v0) <= maxDiff); }

template<typename T,uint nrows,uint ncols>
bool
isApproxEqual(
    Mat<T,nrows,ncols> const &    m0,
    Mat<T,nrows,ncols> const &    m1,
    T                             maxDiff)
{
    for (uint ii=0; ii<nrows*ncols; ++ii)
        if (std::abs(m0[ii]-m1[ii]) > maxDiff)
            return false;
    return true;
}

template<typename T>
uint constexpr defaultPrecisionBits();

template<> uint constexpr defaultPrecisionBits<float>() {return 20;}
template<> uint constexpr defaultPrecisionBits<double>() {return 40;}

// Are two numbers approximately equal relative to their absolute sizes ?
template<typename T,
    FG_ENABLE_IF(T,is_floating_point)>
bool
isApproxEqualRel(T v0,T v1,double maxRelDiff)
{
    double      d0 = scast<double>(v0),
                d1 = scast<double>(v1),
                denom = std::abs(d0) + std::abs(d1);
    if (denom == 0.0)
        return true;
    double      rel = (2.0 * std::abs(d1-d0)) / (std::abs(d0) + std::abs(d1));
    return (rel < maxRelDiff);
}

template<typename T,
    FG_ENABLE_IF(T,is_floating_point)>
bool
isApproxEqualRelPrec(T v0,T v1,uint precisionBits=defaultPrecisionBits<T>())
{
    FGASSERT(precisionBits <= defaultPrecisionBits<T>());   // Close to max
    double      maxRelDiff = 1.0 / scast<double>(1ULL << precisionBits);
    return isApproxEqualRel(v0,v1,maxRelDiff);
}

template<typename T,
    FG_ENABLE_IF(T,is_floating_point)>
bool
isApproxEqualAbsPrec(T v0,T v1,T scale,uint precisionBits=defaultPrecisionBits<T>())
{
    FGASSERT(precisionBits <= defaultPrecisionBits<T>());   // default close to max
    T           precision = T(1) / T(1ULL << precisionBits);
    return isApproxEqual(v0,v1,scale*precision);
}

// Ensure the L2 norms are equal to the given number of bits of precision.
// The arguments must be numerical containers supporting 'cMag' and subtraction:
template<typename Container>
bool
isApproxEqualRelMag(Container const & lhs,Container const & rhs,uint precisionBits=20)
{
    FGASSERT(lhs.size() == rhs.size());
    double          precision = scast<double>(1ULL << precisionBits),
                    precSqr = sqr(precision);
    FGASSERT(precSqr > 0.0);
    double          mag = cMax(cMag(lhs),cMag(rhs));
    if (mag == 0.0)
        return true;
    double          rel = cMag(lhs-rhs) / mag;
    return (rel < precSqr);
}

template<typename T>
bool
isApproxEqualRelPrec(
    MatV<T> const &             lhs,
    MatV<T> const &             rhs,
    uint                        precisionBits=defaultPrecisionBits<T>())
{
    FGASSERT(lhs.dims() == rhs.dims());
    T           scale = cMax(mapAbs(lhs.m_data)) + cMax(mapAbs(rhs.m_data));
    if (scale == 0)
        return true;
    T           maxDiff = scale / T(2ULL << precisionBits);     // 2 gives extra 1/2 factor for 'scale' sum
    for (size_t ii=0; ii<lhs.numElems(); ++ii)
        if (std::abs(rhs[ii]-lhs[ii]) > maxDiff)
            return false;
    return true;
}

template<typename T,uint nrows,uint ncols>
bool
isApproxEqualRelPrec(
    Mat<T,nrows,ncols> const &  lhs,
    Mat<T,nrows,ncols> const &  rhs,
    uint                        precisionBits=defaultPrecisionBits<T>())
{
    T           scale = (cMaxElem(mapAbs(lhs)) + cMaxElem(mapAbs(rhs))) * T(0.5);
    if (scale == 0)
        return true;
    T           maxDiff = scale * bitsToPrecision(precisionBits);
    return isApproxEqual(lhs,rhs,maxDiff);
}

template<typename T,uint nrows,uint ncols>
bool
isApproxEqualRelPrec(
    Svec<Mat<T,nrows,ncols> > const &   lhs,
    Svec<Mat<T,nrows,ncols> > const &   rhs,
    uint                                precisionBits=defaultPrecisionBits<T>())
{
    FGASSERT(lhs.size() == rhs.size());
    T           scale = cMaxElem((mapAbs(cDims(lhs)) + mapAbs(cDims(rhs))) * T(0.5)),
                precision = bitsToPrecision(precisionBits),
                maxDiff = scale * precision;
    for (size_t ii=0; ii<lhs.size(); ++ii)
        if (!isApproxEqual(lhs[ii],rhs[ii],maxDiff))
            return false;
    return true;
}

}

#endif
