/****************************************************************************
** Meta object code from reading C++ file 'mainwindow.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.4.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../editor/mainwindow.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mainwindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.4.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_MainWindow_t {
    QByteArrayData data[18];
    char stringdata[229];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_MainWindow_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_MainWindow_t qt_meta_stringdata_MainWindow = {
    {
QT_MOC_LITERAL(0, 0, 10), // "MainWindow"
QT_MOC_LITERAL(1, 11, 7), // "fileNew"
QT_MOC_LITERAL(2, 19, 0), // ""
QT_MOC_LITERAL(3, 20, 8), // "fileOpen"
QT_MOC_LITERAL(4, 29, 8), // "fileSave"
QT_MOC_LITERAL(5, 38, 10), // "fileSaveAs"
QT_MOC_LITERAL(6, 49, 16), // "fileRemoteExport"
QT_MOC_LITERAL(7, 66, 14), // "openRecentFile"
QT_MOC_LITERAL(8, 81, 8), // "fileQuit"
QT_MOC_LITERAL(9, 90, 17), // "editBiasSelection"
QT_MOC_LITERAL(10, 108, 11), // "editSetRows"
QT_MOC_LITERAL(11, 120, 20), // "editPreviousBookmark"
QT_MOC_LITERAL(12, 141, 16), // "editNextBookmark"
QT_MOC_LITERAL(13, 158, 12), // "onPosChanged"
QT_MOC_LITERAL(14, 171, 14), // "onCurrValDirty"
QT_MOC_LITERAL(15, 186, 11), // "onReadyRead"
QT_MOC_LITERAL(16, 198, 15), // "onNewConnection"
QT_MOC_LITERAL(17, 214, 14) // "onDisconnected"

    },
    "MainWindow\0fileNew\0\0fileOpen\0fileSave\0"
    "fileSaveAs\0fileRemoteExport\0openRecentFile\0"
    "fileQuit\0editBiasSelection\0editSetRows\0"
    "editPreviousBookmark\0editNextBookmark\0"
    "onPosChanged\0onCurrValDirty\0onReadyRead\0"
    "onNewConnection\0onDisconnected"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MainWindow[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      16,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   94,    2, 0x0a /* Public */,
       3,    0,   95,    2, 0x0a /* Public */,
       4,    0,   96,    2, 0x0a /* Public */,
       5,    0,   97,    2, 0x0a /* Public */,
       6,    0,   98,    2, 0x0a /* Public */,
       7,    0,   99,    2, 0x0a /* Public */,
       8,    0,  100,    2, 0x0a /* Public */,
       9,    0,  101,    2, 0x0a /* Public */,
      10,    0,  102,    2, 0x0a /* Public */,
      11,    0,  103,    2, 0x0a /* Public */,
      12,    0,  104,    2, 0x0a /* Public */,
      13,    0,  105,    2, 0x0a /* Public */,
      14,    0,  106,    2, 0x0a /* Public */,
      15,    0,  107,    2, 0x08 /* Private */,
      16,    0,  108,    2, 0x08 /* Private */,
      17,    0,  109,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
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

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        MainWindow *_t = static_cast<MainWindow *>(_o);
        switch (_id) {
        case 0: _t->fileNew(); break;
        case 1: _t->fileOpen(); break;
        case 2: _t->fileSave(); break;
        case 3: _t->fileSaveAs(); break;
        case 4: _t->fileRemoteExport(); break;
        case 5: _t->openRecentFile(); break;
        case 6: _t->fileQuit(); break;
        case 7: _t->editBiasSelection(); break;
        case 8: _t->editSetRows(); break;
        case 9: _t->editPreviousBookmark(); break;
        case 10: _t->editNextBookmark(); break;
        case 11: _t->onPosChanged(); break;
        case 12: _t->onCurrValDirty(); break;
        case 13: _t->onReadyRead(); break;
        case 14: _t->onNewConnection(); break;
        case 15: _t->onDisconnected(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObject MainWindow::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_MainWindow.data,
      qt_meta_data_MainWindow,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_MainWindow.stringdata))
        return static_cast<void*>(const_cast< MainWindow*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 16)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 16;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 16)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 16;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
