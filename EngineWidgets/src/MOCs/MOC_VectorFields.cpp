/****************************************************************************
** Meta object code from reading C++ file 'VectorFields.hpp'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.8.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../VectorFields.hpp"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'VectorFields.hpp' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.8.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {

#ifdef QT_MOC_HAS_STRINGDATA
struct qt_meta_stringdata_CLASScpSCOPEFloat2ENDCLASS_t {};
constexpr auto qt_meta_stringdata_CLASScpSCOPEFloat2ENDCLASS = QtMocHelpers::stringData(
    "cp::Float2"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASScpSCOPEFloat2ENDCLASS[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

Q_CONSTINIT const QMetaObject cp::Float2::staticMetaObject = { {
    QMetaObject::SuperData::link<cp::VectorWidget<glm::vec<2,glm::f32,glm::defaultp>,2>::staticMetaObject>(),
    qt_meta_stringdata_CLASScpSCOPEFloat2ENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASScpSCOPEFloat2ENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASScpSCOPEFloat2ENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<Float2, std::true_type>
    >,
    nullptr
} };

void cp::Float2::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    (void)_o;
    (void)_id;
    (void)_c;
    (void)_a;
}

const QMetaObject *cp::Float2::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *cp::Float2::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASScpSCOPEFloat2ENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return cp::VectorWidget<glm::vec<2,glm::f32,glm::defaultp>,2>::qt_metacast(_clname);
}

int cp::Float2::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = cp::VectorWidget<glm::vec<2,glm::f32,glm::defaultp>,2>::qt_metacall(_c, _id, _a);
    return _id;
}
namespace {

#ifdef QT_MOC_HAS_STRINGDATA
struct qt_meta_stringdata_CLASScpSCOPEFloat3ENDCLASS_t {};
constexpr auto qt_meta_stringdata_CLASScpSCOPEFloat3ENDCLASS = QtMocHelpers::stringData(
    "cp::Float3"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASScpSCOPEFloat3ENDCLASS[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

Q_CONSTINIT const QMetaObject cp::Float3::staticMetaObject = { {
    QMetaObject::SuperData::link<cp::VectorWidget<glm::vec<3,glm::f32,glm::defaultp>,3>::staticMetaObject>(),
    qt_meta_stringdata_CLASScpSCOPEFloat3ENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASScpSCOPEFloat3ENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASScpSCOPEFloat3ENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<Float3, std::true_type>
    >,
    nullptr
} };

void cp::Float3::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    (void)_o;
    (void)_id;
    (void)_c;
    (void)_a;
}

const QMetaObject *cp::Float3::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *cp::Float3::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASScpSCOPEFloat3ENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return cp::VectorWidget<glm::vec<3,glm::f32,glm::defaultp>,3>::qt_metacast(_clname);
}

int cp::Float3::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = cp::VectorWidget<glm::vec<3,glm::f32,glm::defaultp>,3>::qt_metacall(_c, _id, _a);
    return _id;
}
namespace {

#ifdef QT_MOC_HAS_STRINGDATA
struct qt_meta_stringdata_CLASScpSCOPEFloat4ENDCLASS_t {};
constexpr auto qt_meta_stringdata_CLASScpSCOPEFloat4ENDCLASS = QtMocHelpers::stringData(
    "cp::Float4"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASScpSCOPEFloat4ENDCLASS[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

Q_CONSTINIT const QMetaObject cp::Float4::staticMetaObject = { {
    QMetaObject::SuperData::link<cp::VectorWidget<glm::vec<4,glm::f32,glm::defaultp>,4>::staticMetaObject>(),
    qt_meta_stringdata_CLASScpSCOPEFloat4ENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASScpSCOPEFloat4ENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASScpSCOPEFloat4ENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<Float4, std::true_type>
    >,
    nullptr
} };

void cp::Float4::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    (void)_o;
    (void)_id;
    (void)_c;
    (void)_a;
}

const QMetaObject *cp::Float4::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *cp::Float4::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASScpSCOPEFloat4ENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return cp::VectorWidget<glm::vec<4,glm::f32,glm::defaultp>,4>::qt_metacast(_clname);
}

int cp::Float4::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = cp::VectorWidget<glm::vec<4,glm::f32,glm::defaultp>,4>::qt_metacall(_c, _id, _a);
    return _id;
}
QT_WARNING_POP
