/****************************************************************************
** Meta object code from reading C++ file 'calibrateviewmanager.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "ViewMgr/calibrateviewmanager.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'calibrateviewmanager.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_GUI__CalibrateViewManager[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       6,       // signalCount

 // signals: signature, parameters, type, tag, flags
      27,   26,   26,   26, 0x05,
      48,   26,   26,   26, 0x05,
      70,   26,   26,   26, 0x05,
      86,   26,   26,   26, 0x05,
     108,  101,   26,   26, 0x05,
     131,  101,   26,   26, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_GUI__CalibrateViewManager[] = {
    "GUI::CalibrateViewManager\0\0"
    "startedCalibration()\0finishedCalibration()\0"
    "disableLaunch()\0enableLaunch()\0enable\0"
    "sendGlobalEnable(bool)\0receiveGlobalEnable(bool)\0"
};

void GUI::CalibrateViewManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        CalibrateViewManager *_t = static_cast<CalibrateViewManager *>(_o);
        switch (_id) {
        case 0: _t->startedCalibration(); break;
        case 1: _t->finishedCalibration(); break;
        case 2: _t->disableLaunch(); break;
        case 3: _t->enableLaunch(); break;
        case 4: _t->sendGlobalEnable((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 5: _t->receiveGlobalEnable((*reinterpret_cast< bool(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData GUI::CalibrateViewManager::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject GUI::CalibrateViewManager::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_GUI__CalibrateViewManager,
      qt_meta_data_GUI__CalibrateViewManager, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &GUI::CalibrateViewManager::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *GUI::CalibrateViewManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *GUI::CalibrateViewManager::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_GUI__CalibrateViewManager))
        return static_cast<void*>(const_cast< CalibrateViewManager*>(this));
    return QObject::qt_metacast(_clname);
}

int GUI::CalibrateViewManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void GUI::CalibrateViewManager::startedCalibration()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void GUI::CalibrateViewManager::finishedCalibration()
{
    QMetaObject::activate(this, &staticMetaObject, 1, 0);
}

// SIGNAL 2
void GUI::CalibrateViewManager::disableLaunch()
{
    QMetaObject::activate(this, &staticMetaObject, 2, 0);
}

// SIGNAL 3
void GUI::CalibrateViewManager::enableLaunch()
{
    QMetaObject::activate(this, &staticMetaObject, 3, 0);
}

// SIGNAL 4
void GUI::CalibrateViewManager::sendGlobalEnable(bool _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void GUI::CalibrateViewManager::receiveGlobalEnable(bool _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}
QT_END_MOC_NAMESPACE
