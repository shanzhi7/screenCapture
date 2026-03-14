/****************************************************************************
** Meta object code from reading C++ file 'selectionoverlay.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.5.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../selectionoverlay.h"
#include <QtCore/qmetatype.h>

#if __has_include(<QtCore/qtmochelpers.h>)
#include <QtCore/qtmochelpers.h>
#else
QT_BEGIN_MOC_NAMESPACE
#endif


#include <memory>

#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'selectionoverlay.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_CLASSSelectionOverlayENDCLASS_t {};
static constexpr auto qt_meta_stringdata_CLASSSelectionOverlayENDCLASS = QtMocHelpers::stringData(
    "SelectionOverlay",
    "selectionFinished",
    "",
    "rect",
    "selectionCanceled",
    "saveRequested",
    "colorValueCopied",
    "colorText",
    "longCaptureToggled",
    "enabled",
    "longCaptureWheel",
    "delta",
    "longCaptureSaveRequested",
    "longCaptureConfirmRequested"
);
#else  // !QT_MOC_HAS_STRING_DATA
struct qt_meta_stringdata_CLASSSelectionOverlayENDCLASS_t {
    uint offsetsAndSizes[28];
    char stringdata0[17];
    char stringdata1[18];
    char stringdata2[1];
    char stringdata3[5];
    char stringdata4[18];
    char stringdata5[14];
    char stringdata6[17];
    char stringdata7[10];
    char stringdata8[19];
    char stringdata9[8];
    char stringdata10[17];
    char stringdata11[6];
    char stringdata12[25];
    char stringdata13[28];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CLASSSelectionOverlayENDCLASS_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CLASSSelectionOverlayENDCLASS_t qt_meta_stringdata_CLASSSelectionOverlayENDCLASS = {
    {
        QT_MOC_LITERAL(0, 16),  // "SelectionOverlay"
        QT_MOC_LITERAL(17, 17),  // "selectionFinished"
        QT_MOC_LITERAL(35, 0),  // ""
        QT_MOC_LITERAL(36, 4),  // "rect"
        QT_MOC_LITERAL(41, 17),  // "selectionCanceled"
        QT_MOC_LITERAL(59, 13),  // "saveRequested"
        QT_MOC_LITERAL(73, 16),  // "colorValueCopied"
        QT_MOC_LITERAL(90, 9),  // "colorText"
        QT_MOC_LITERAL(100, 18),  // "longCaptureToggled"
        QT_MOC_LITERAL(119, 7),  // "enabled"
        QT_MOC_LITERAL(127, 16),  // "longCaptureWheel"
        QT_MOC_LITERAL(144, 5),  // "delta"
        QT_MOC_LITERAL(150, 24),  // "longCaptureSaveRequested"
        QT_MOC_LITERAL(175, 27)   // "longCaptureConfirmRequested"
    },
    "SelectionOverlay",
    "selectionFinished",
    "",
    "rect",
    "selectionCanceled",
    "saveRequested",
    "colorValueCopied",
    "colorText",
    "longCaptureToggled",
    "enabled",
    "longCaptureWheel",
    "delta",
    "longCaptureSaveRequested",
    "longCaptureConfirmRequested"
};
#undef QT_MOC_LITERAL
#endif // !QT_MOC_HAS_STRING_DATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSSelectionOverlayENDCLASS[] = {

 // content:
      11,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       8,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,   62,    2, 0x06,    1 /* Public */,
       4,    0,   65,    2, 0x06,    3 /* Public */,
       5,    1,   66,    2, 0x06,    4 /* Public */,
       6,    1,   69,    2, 0x06,    6 /* Public */,
       8,    2,   72,    2, 0x06,    8 /* Public */,
      10,    2,   77,    2, 0x06,   11 /* Public */,
      12,    1,   82,    2, 0x06,   14 /* Public */,
      13,    1,   85,    2, 0x06,   16 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QRect,    3,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QRect,    3,
    QMetaType::Void, QMetaType::QString,    7,
    QMetaType::Void, QMetaType::Bool, QMetaType::QRect,    9,    3,
    QMetaType::Void, QMetaType::QRect, QMetaType::Int,    3,   11,
    QMetaType::Void, QMetaType::QRect,    3,
    QMetaType::Void, QMetaType::QRect,    3,

       0        // eod
};

Q_CONSTINIT const QMetaObject SelectionOverlay::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_CLASSSelectionOverlayENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSSelectionOverlayENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSSelectionOverlayENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<SelectionOverlay, std::true_type>,
        // method 'selectionFinished'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QRect &, std::false_type>,
        // method 'selectionCanceled'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'saveRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QRect &, std::false_type>,
        // method 'colorValueCopied'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'longCaptureToggled'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QRect &, std::false_type>,
        // method 'longCaptureWheel'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QRect &, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'longCaptureSaveRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QRect &, std::false_type>,
        // method 'longCaptureConfirmRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QRect &, std::false_type>
    >,
    nullptr
} };

void SelectionOverlay::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<SelectionOverlay *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->selectionFinished((*reinterpret_cast< std::add_pointer_t<QRect>>(_a[1]))); break;
        case 1: _t->selectionCanceled(); break;
        case 2: _t->saveRequested((*reinterpret_cast< std::add_pointer_t<QRect>>(_a[1]))); break;
        case 3: _t->colorValueCopied((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 4: _t->longCaptureToggled((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QRect>>(_a[2]))); break;
        case 5: _t->longCaptureWheel((*reinterpret_cast< std::add_pointer_t<QRect>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 6: _t->longCaptureSaveRequested((*reinterpret_cast< std::add_pointer_t<QRect>>(_a[1]))); break;
        case 7: _t->longCaptureConfirmRequested((*reinterpret_cast< std::add_pointer_t<QRect>>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (SelectionOverlay::*)(const QRect & );
            if (_t _q_method = &SelectionOverlay::selectionFinished; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (SelectionOverlay::*)();
            if (_t _q_method = &SelectionOverlay::selectionCanceled; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (SelectionOverlay::*)(const QRect & );
            if (_t _q_method = &SelectionOverlay::saveRequested; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (SelectionOverlay::*)(const QString & );
            if (_t _q_method = &SelectionOverlay::colorValueCopied; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (SelectionOverlay::*)(bool , const QRect & );
            if (_t _q_method = &SelectionOverlay::longCaptureToggled; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (SelectionOverlay::*)(const QRect & , int );
            if (_t _q_method = &SelectionOverlay::longCaptureWheel; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 5;
                return;
            }
        }
        {
            using _t = void (SelectionOverlay::*)(const QRect & );
            if (_t _q_method = &SelectionOverlay::longCaptureSaveRequested; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 6;
                return;
            }
        }
        {
            using _t = void (SelectionOverlay::*)(const QRect & );
            if (_t _q_method = &SelectionOverlay::longCaptureConfirmRequested; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 7;
                return;
            }
        }
    }
}

const QMetaObject *SelectionOverlay::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *SelectionOverlay::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSSelectionOverlayENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int SelectionOverlay::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 8)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void SelectionOverlay::selectionFinished(const QRect & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void SelectionOverlay::selectionCanceled()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void SelectionOverlay::saveRequested(const QRect & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void SelectionOverlay::colorValueCopied(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void SelectionOverlay::longCaptureToggled(bool _t1, const QRect & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void SelectionOverlay::longCaptureWheel(const QRect & _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void SelectionOverlay::longCaptureSaveRequested(const QRect & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}

// SIGNAL 7
void SelectionOverlay::longCaptureConfirmRequested(const QRect & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 7, _a);
}
QT_WARNING_POP
