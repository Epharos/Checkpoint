/****************************************************************************
** Meta object code from reading C++ file 'Launcher.hpp'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.8.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "pch.hpp"

#include "Launcher.hpp"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'Launcher.hpp' doesn't include <QObject>."
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
struct qt_meta_stringdata_CLASSLauncherENDCLASS_t {};
constexpr auto qt_meta_stringdata_CLASSLauncherENDCLASS = QtMocHelpers::stringData(
    "Launcher",
    "OpenProject",
    "",
    "std::string",
    "_path",
    "ShowProjectData",
    "CreateNewProject",
    "SaveNewProject",
    "ProjectData",
    "projectData",
    "SaveRecentProjects",
    "std::vector<ProjectData>",
    "projects",
    "LoadRecentProjects"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSLauncherENDCLASS[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,   50,    2, 0x08,    1 /* Private */,
       5,    1,   53,    2, 0x08,    3 /* Private */,
       6,    0,   56,    2, 0x08,    5 /* Private */,
       7,    1,   57,    2, 0x08,    6 /* Private */,
      10,    1,   60,    2, 0x08,    8 /* Private */,
      13,    0,   63,    2, 0x08,   10 /* Private */,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 8,    9,
    QMetaType::Void, 0x80000000 | 11,   12,
    0x80000000 | 11,

       0        // eod
};

Q_CONSTINIT const QMetaObject Launcher::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_CLASSLauncherENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSLauncherENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSLauncherENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<Launcher, std::true_type>,
        // method 'OpenProject'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const std::string &, std::false_type>,
        // method 'ShowProjectData'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const std::string &, std::false_type>,
        // method 'CreateNewProject'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'SaveNewProject'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const ProjectData &, std::false_type>,
        // method 'SaveRecentProjects'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const std::vector<ProjectData> &, std::false_type>,
        // method 'LoadRecentProjects'
        QtPrivate::TypeAndForceComplete<std::vector<ProjectData>, std::false_type>
    >,
    nullptr
} };

void Launcher::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<Launcher *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->OpenProject((*reinterpret_cast< std::add_pointer_t<std::string>>(_a[1]))); break;
        case 1: _t->ShowProjectData((*reinterpret_cast< std::add_pointer_t<std::string>>(_a[1]))); break;
        case 2: _t->CreateNewProject(); break;
        case 3: _t->SaveNewProject((*reinterpret_cast< std::add_pointer_t<ProjectData>>(_a[1]))); break;
        case 4: _t->SaveRecentProjects((*reinterpret_cast< std::add_pointer_t<std::vector<ProjectData>>>(_a[1]))); break;
        case 5: { std::vector<ProjectData> _r = _t->LoadRecentProjects();
            if (_a[0]) *reinterpret_cast< std::vector<ProjectData>*>(_a[0]) = std::move(_r); }  break;
        default: ;
        }
    }
}

const QMetaObject *Launcher::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Launcher::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSLauncherENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int Launcher::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 6;
    }
    return _id;
}
QT_WARNING_POP
