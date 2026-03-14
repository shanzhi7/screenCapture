/****************************************************************************
** Meta object code from reading C++ file 'longcapturesessioncontroller.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.5.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../longcapturesessioncontroller.h"
#include <QtCore/qmetatype.h>

#if __has_include(<QtCore/qtmochelpers.h>)
#include <QtCore/qtmochelpers.h>
#else
QT_BEGIN_MOC_NAMESPACE
#endif


#include <memory>

#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'longcapturesessioncontroller.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.5.3. It"
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
struct qt_meta_stringdata_CLASSLongCaptureSessionControllerENDCLASS_t {};
static constexpr auto qt_meta_stringdata_CLASSLongCaptureSessionControllerENDCLASS = QtMocHelpers::stringData(
    "LongCaptureSessionController",
    "previewUpdated",
    "",
    "pixmap",
    "predictedVisualHeightChanged",
    "height",
    "committedVisualHeightChanged",
    "captureQualityChanged",
    "CaptureQuality",
    "quality",
    "copyReady",
    "saveReady",
    "failed",
    "message",
    "statusTextChanged",
    "onObserveTimeout"
);
#else  // !QT_MOC_HAS_STRING_DATA
struct qt_meta_stringdata_CLASSLongCaptureSessionControllerENDCLASS_t {
    uint offsetsAndSizes[32];
    char stringdata0[29];
    char stringdata1[15];
    char stringdata2[1];
    char stringdata3[7];
    char stringdata4[29];
    char stringdata5[7];
    char stringdata6[29];
    char stringdata7[22];
    char stringdata8[15];
    char stringdata9[8];
    char stringdata10[10];
    char stringdata11[10];
    char stringdata12[7];
    char stringdata13[8];
    char stringdata14[18];
    char stringdata15[17];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CLASSLongCaptureSessionControllerENDCLASS_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CLASSLongCaptureSessionControllerENDCLASS_t qt_meta_stringdata_CLASSLongCaptureSessionControllerENDCLASS = {
    {
        QT_MOC_LITERAL(0, 28),  // "LongCaptureSessionController"
        QT_MOC_LITERAL(29, 14),  // "previewUpdated"
        QT_MOC_LITERAL(44, 0),  // ""
        QT_MOC_LITERAL(45, 6),  // "pixmap"
        QT_MOC_LITERAL(52, 28),  // "predictedVisualHeightChanged"
        QT_MOC_LITERAL(81, 6),  // "height"
        QT_MOC_LITERAL(88, 28),  // "committedVisualHeightChanged"
        QT_MOC_LITERAL(117, 21),  // "captureQualityChanged"
        QT_MOC_LITERAL(139, 14),  // "CaptureQuality"
        QT_MOC_LITERAL(154, 7),  // "quality"
        QT_MOC_LITERAL(162, 9),  // "copyReady"
        QT_MOC_LITERAL(172, 9),  // "saveReady"
        QT_MOC_LITERAL(182, 6),  // "failed"
        QT_MOC_LITERAL(189, 7),  // "message"
        QT_MOC_LITERAL(197, 17),  // "statusTextChanged"
        QT_MOC_LITERAL(215, 16)   // "onObserveTimeout"
    },
    "LongCaptureSessionController",
    "previewUpdated",
    "",
    "pixmap",
    "predictedVisualHeightChanged",
    "height",
    "committedVisualHeightChanged",
    "captureQualityChanged",
    "CaptureQuality",
    "quality",
    "copyReady",
    "saveReady",
    "failed",
    "message",
    "statusTextChanged",
    "onObserveTimeout"
};
#undef QT_MOC_LITERAL
#endif // !QT_MOC_HAS_STRING_DATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSLongCaptureSessionControllerENDCLASS[] = {

 // content:
      11,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       8,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,   68,    2, 0x06,    1 /* Public */,
       4,    1,   71,    2, 0x06,    3 /* Public */,
       6,    1,   74,    2, 0x06,    5 /* Public */,
       7,    1,   77,    2, 0x06,    7 /* Public */,
      10,    1,   80,    2, 0x06,    9 /* Public */,
      11,    1,   83,    2, 0x06,   11 /* Public */,
      12,    1,   86,    2, 0x06,   13 /* Public */,
      14,    1,   89,    2, 0x06,   15 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
      15,    0,   92,    2, 0x08,   17 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::QPixmap,    3,
    QMetaType::Void, QMetaType::Int,    5,
    QMetaType::Void, QMetaType::Int,    5,
    QMetaType::Void, 0x80000000 | 8,    9,
    QMetaType::Void, QMetaType::QPixmap,    3,
    QMetaType::Void, QMetaType::QPixmap,    3,
    QMetaType::Void, QMetaType::QString,   13,
    QMetaType::Void, QMetaType::QString,   13,

 // slots: parameters
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject LongCaptureSessionController::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_CLASSLongCaptureSessionControllerENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSLongCaptureSessionControllerENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSLongCaptureSessionControllerENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<LongCaptureSessionController, std::true_type>,
        // method 'previewUpdated'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QPixmap &, std::false_type>,
        // method 'predictedVisualHeightChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'committedVisualHeightChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'captureQualityChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<CaptureQuality, std::false_type>,
        // method 'copyReady'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QPixmap &, std::false_type>,
        // method 'saveReady'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QPixmap &, std::false_type>,
        // method 'failed'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'statusTextChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'onObserveTimeout'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void LongCaptureSessionController::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<LongCaptureSessionController *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->previewUpdated((*reinterpret_cast< std::add_pointer_t<QPixmap>>(_a[1]))); break;
        case 1: _t->predictedVisualHeightChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 2: _t->committedVisualHeightChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 3: _t->captureQualityChanged((*reinterpret_cast< std::add_pointer_t<CaptureQuality>>(_a[1]))); break;
        case 4: _t->copyReady((*reinterpret_cast< std::add_pointer_t<QPixmap>>(_a[1]))); break;
        case 5: _t->saveReady((*reinterpret_cast< std::add_pointer_t<QPixmap>>(_a[1]))); break;
        case 6: _t->failed((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 7: _t->statusTextChanged((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 8: _t->onObserveTimeout(); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 3:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< CaptureQuality >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (LongCaptureSessionController::*)(const QPixmap & );
            if (_t _q_method = &LongCaptureSessionController::previewUpdated; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (LongCaptureSessionController::*)(int );
            if (_t _q_method = &LongCaptureSessionController::predictedVisualHeightChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (LongCaptureSessionController::*)(int );
            if (_t _q_method = &LongCaptureSessionController::committedVisualHeightChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (LongCaptureSessionController::*)(CaptureQuality );
            if (_t _q_method = &LongCaptureSessionController::captureQualityChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (LongCaptureSessionController::*)(const QPixmap & );
            if (_t _q_method = &LongCaptureSessionController::copyReady; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (LongCaptureSessionController::*)(const QPixmap & );
            if (_t _q_method = &LongCaptureSessionController::saveReady; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 5;
                return;
            }
        }
        {
            using _t = void (LongCaptureSessionController::*)(const QString & );
            if (_t _q_method = &LongCaptureSessionController::failed; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 6;
                return;
            }
        }
        {
            using _t = void (LongCaptureSessionController::*)(const QString & );
            if (_t _q_method = &LongCaptureSessionController::statusTextChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 7;
                return;
            }
        }
    }
}

const QMetaObject *LongCaptureSessionController::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *LongCaptureSessionController::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSLongCaptureSessionControllerENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int LongCaptureSessionController::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    }
    return _id;
}

// SIGNAL 0
void LongCaptureSessionController::previewUpdated(const QPixmap & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void LongCaptureSessionController::predictedVisualHeightChanged(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void LongCaptureSessionController::committedVisualHeightChanged(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void LongCaptureSessionController::captureQualityChanged(CaptureQuality _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void LongCaptureSessionController::copyReady(const QPixmap & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void LongCaptureSessionController::saveReady(const QPixmap & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void LongCaptureSessionController::failed(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}

// SIGNAL 7
void LongCaptureSessionController::statusTextChanged(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 7, _a);
}
QT_WARNING_POP
