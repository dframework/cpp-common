#ifndef DFRAMEWORK_UTIL_D_SORTED_H
#define DFRAMEWORK_UTIL_D_SORTED_H
#define DFRAMEWORK_UTIL_D_SORTED_VERSION 1


#define DFW_D_VBO(_op_, _c_, _r_) \
    virtual bool operator _op_ (const _c_ _r_ p)

#define DFW_D_IVBO(_op_, _c_, _r_) \
    inline virtual bool operator _op_ (const _c_ _r_ p)

#define DFW_OPERATOR_POINT_DECLARATION(c) \
    DFW_D_VBO(==,c,*); \
    DFW_D_VBO(!=,c,*); \
    DFW_D_VBO(> ,c,*); \
    DFW_D_VBO(< ,c,*); \
    DFW_D_VBO(>=,c,*); \
    DFW_D_VBO(<=,c,*);

#define DFW_OPERATOR_REF_DECLARATION(c) \
    DFW_D_VBO(==,c,&); \
    DFW_D_VBO(!=,c,&); \
    DFW_D_VBO(> ,c,&); \
    DFW_D_VBO(< ,c,&); \
    DFW_D_VBO(>=,c,&); \
    DFW_D_VBO(<=,c,&);

#define DFW_OPERATOR_DECLARATION(c)   \
    DFW_OPERATOR_POINT_DECLARATION(c) \
    DFW_OPERATOR_REF_DECLARATION(c)

#define DFW_OPERATOR_EX_POINT_DECLARATION(c)                    \
    DFW_D_IVBO(==,c,*) { return (!p ? false : (*this == *p)); } \
    DFW_D_IVBO(!=,c,*) { return (!p ? true  : (*this != *p)); } \
    DFW_D_IVBO(> ,c,*) { return (!p ? true  : (*this >  *p)); } \
    DFW_D_IVBO(< ,c,*) { return (!p ? false : (*this <  *p)); } \
    DFW_D_IVBO(>=,c,*) { return (!p ? true  : (*this >= *p)); } \
    DFW_D_IVBO(<=,c,*) { return (!p ? false : (*this <= *p)); }

#define DFW_OPERATOR_EX_REF_DECLARATION(c,m)  \
    DFW_D_IVBO(==,c,&) { return (m == p.m);}  \
    DFW_D_IVBO(!=,c,&) { return (m != p.m);}  \
    DFW_D_IVBO(> ,c,&) { return (m >  p.m);}  \
    DFW_D_IVBO(< ,c,&) { return (m <  p.m);}  \
    DFW_D_IVBO(>=,c,&) { return (m >= p.m);}  \
    DFW_D_IVBO(<=,c,&) { return (m <= p.m);}

#define DFW_OPERATOR_EX_DECLARATION(c,m)      \
    DFW_OPERATOR_EX_POINT_DECLARATION(c)      \
    DFW_OPERATOR_EX_REF_DECLARATION(c,m)

#define DFW_OPERATOR_EXP_DECLARATION(c)       \
    DFW_OPERATOR_EX_POINT_DECLARATION(c)      \
    DFW_OPERATOR_REF_DECLARATION(c)

// ------------------------------------------------------------------

#define DFW_D_TL_IVBO(_op_, _c_, _r_) \
    inline bool operator _op_ (const _c_ _r_ p)

#define DFW_OPERATOR_TL_EX_HALF_DECLARATION(c)        \
    DFW_D_TL_IVBO(==,c,&) { return (!(*this != p)); } \
    DFW_D_TL_IVBO(> ,c,&) { return (!(*this <= p)); } \
    DFW_D_TL_IVBO(< ,c,&) { return (!(*this >= p)); }

#define DFW_OPERATOR_TL_EX_DECLARATION(c,m)                          \
    DFW_OPERATOR_EX_POINT_DECLARATION(c)                             \
    DFW_OPERATOR_TL_EX_HALF_DECLARATION(c)                           \
    DFW_D_TL_IVBO(!=,c,&) {                                          \
        return (!m&&!p.m?false:(!m||!p.m?true:(*m!=*p.m))); }        \
    DFW_D_TL_IVBO(>=,c,&) {                                          \
        return (!m&&!p.m?true:(!m ?false:(!p.m?true:(*m>=*p.m)))); } \
    DFW_D_TL_IVBO(<=,c,&) {                                          \
        return (!m&&!p.m?true:(!m ?true:(!p.m?false:(*m<=*p.m)))); }



#endif /* DFRAMEWORK_UTIL_D_SORTED_H */

