/****************************************************************************
** Meta object code from reading C++ file 'trackview.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.4.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../editor/trackview.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'trackview.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.4.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_TrackView_t {
    QByteArrayData data[16];
    char stringdata[148];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_TrackView_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_TrackView_t qt_meta_stringdata_TrackView = {
    {
QT_MOC_LITERAL(0, 0, 9), // "TrackView"
QT_MOC_LITERAL(1, 10, 10), // "posChanged"
QT_MOC_LITERAL(2, 21, 0), // ""
QT_MOC_LITERAL(3, 22, 12), // "currValDirty"
QT_MOC_LITERAL(4, 35, 9), // "onHScroll"
QT_MOC_LITERAL(5, 45, 5), // "value"
QT_MOC_LITERAL(6, 51, 9), // "onVScroll"
QT_MOC_LITERAL(7, 61, 8), // "editUndo"
QT_MOC_LITERAL(8, 70, 8), // "editRedo"
QT_MOC_LITERAL(9, 79, 8), // "editCopy"
QT_MOC_LITERAL(10, 88, 7), // "editCut"
QT_MOC_LITERAL(11, 96, 9), // "editPaste"
QT_MOC_LITERAL(12, 106, 9), // "editClear"
QT_MOC_LITERAL(13, 116, 9), // "selectAll"
QT_MOC_LITERAL(14, 126, 11), // "selectTrack"
QT_MOC_LITERAL(15, 138, 9) // "selectRow"

    },
    "TrackView\0posChanged\0\0currValDirty\0"
    "onHScroll\0value\0onVScroll\0editUndo\0"
    "editRedo\0editCopy\0editCut\0editPaste\0"
    "editClear\0selectAll\0selectTrack\0"
    "selectRow"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_TrackView[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      13,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   79,    2, 0x06 /* Public */,
       3,    0,   80,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       4,    1,   81,    2, 0x08 /* Private */,
       6,    1,   84,    2, 0x08 /* Private */,
       7,    0,   87,    2, 0x0a /* Public */,
       8,    0,   88,    2, 0x0a /* Public */,
       9,    0,   89,    2, 0x0a /* Public */,
      10,    0,   90,    2, 0x0a /* Public */,
      11,    0,   91,    2, 0x0a /* Public */,
      12,    0,   92,    2, 0x0a /* Public */,
      13,    0,   93,    2, 0x0a /* Public */,
      14,    0,   94,    2, 0x0a /* Public */,
      15,    0,   95,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    5,
    QMetaType::Void, QMetaType::Int,    5,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void TrackView::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        TrackView *_t = static_cast<TrackView *>(_o);
        switch (_id) {
        case 0: _t->posChanged(); break;
        case 1: _t->currValDirty(); break;
        case 2: _t->onHScroll((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->onVScroll((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->editUndo(); break;
        case 5: _t->editRedo(); break;
        case 6: _t->editCopy(); break;
        case 7: _t->editCut(); break;
        case 8: _t->editPaste(); break;
        case 9: _t->editClear(); break;
        case 10: _t->selectAll(); break;
        case 11: _t->selectTrack(); break;
        case 12: _t->selectRow(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (TrackView::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&TrackView::posChanged)) {
                *result = 0;
            }
        }
        {
            typedef void (TrackView::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&TrackView::currValDirty)) {
                *result = 1;
            }
        }
    }
}

const QMetaObject TrackView::staticMetaObject = {
    { &QAbstractScrollArea::staticMetaObject, qt_meta_stringdata_TrackView.data,
      qt_meta_data_TrackView,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *TrackView::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *TrackView::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_TrackView.stringdata))
        return static_cast<void*>(const_cast< TrackView*>(this));
    return QAbstractScrollArea::qt_metacast(_clname);
}

int TrackView::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QAbstractScrollArea::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 13)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 13;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 13)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 13;
    }
    return _id;
}

// SIGNAL 0
void TrackView::posChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, Q_NULLPTR);
}

// SIGNAL 1
void TrackView::currValDirty()
{
    QMetaObject::activate(this, &staticMetaObject, 1, Q_NULLPTR);
}
QT_END_MOC_NAMESPACE
