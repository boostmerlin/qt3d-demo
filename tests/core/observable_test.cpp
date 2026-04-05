//
// Created by BoostMerlin
//


#include <iostream>
#include <doctest/doctest.h>
#include "test_helper.h"
#include "core/x_property_validator.h"
#include "core/observable_object.h"

class XValidatorListContain : public XValidator<QList<int>>{
public:
    XValidatorListContain() {
        BEGIN_VALIDATE(QList<int>, value, _)
            if (value.contains(3)) {
                return true;
            }
            return false;
        END_VALIDATE
    }
};

class ValidatorTest final : public QObservableObject {
    Q_OBJECT
    X_PROPERTY(int, a, 0, getA, setA)
    X_PROPERTY(int, b, getB, setB)
    X_PROPERTY(QList<int>, List)
    X_META_BEGIN
        X_VALIDATOR(b, (XVLess<int>(10) | XVGreater(20)) & !XVEqual(5))
        X_VALIDATOR(List, XValidatorListContain())
    X_META_END
public:
    ValidatorTest() {
        m_a.setValidator(XVRange<double,true>(5.0, 10.0));
    }
};

class DeleterTest final : public QObservableObject {
Q_OBJECT
X_PROPERTY(int*, a)
X_PROPERTY(int*, b)
X_PROPERTY(QList<int>, List)
X_META_BEGIN
        X_DELETER(a)
        X_DELETER(b, ArrayDeleter<int*>)
        X_DELETER(List, ListClear)
X_META_END
    static void ListClear(QList<int> &list) {
        list.clear();
    }
};


class B : public QObservableObject {
    Q_OBJECT
    X_PROPERTY(int, B);
};

namespace NS1 {
    class A : public QObservableObject {
        Q_OBJECT

        X_PROPERTY(int, a, 0, getA, setA);
        X_PROPERTY(int, b, 0, getB, setB);
        X_PROPERTY(B*, pb, getPB, setPB);

    public:
        void emitValueChange() {
            emit aChanged(m_a, "hello", new A, "world");
        }

        signals:

            void aChanged(int, QString, const A *, const QString &);
    };
}
using namespace NS1;
using IntAMap = QMap<int, A *>;
using IntAHash = QHash<int, A *>;
template<typename T1, typename T2>
using QPair = std::pair<T1, T2>;

using APair = QPair<A, A>; //can't use like this, because QObject is not copyable
using APair2 = QPair<A *, A *>;
using IntPair3 = QPair<int, int>;

class B_Pointer : public QObservableObject {
Q_OBJECT

X_PROPERTY(IntAMap*, PMap);
};

class B_Map : public QObservableObject {
Q_OBJECT

X_PROPERTY(IntAMap, MapA);
};

class B_Hash : public QObservableObject {
Q_OBJECT

X_PROPERTY(IntAHash, Hash);
};

class B_List : public QObservableObject {
Q_OBJECT

X_PROPERTY(QList<IntAHash>, List);
};

class B_Pair : public QObservableObject {
Q_OBJECT

X_PROPERTY(APair2, Pair2);
X_PROPERTY(IntPair3, Pair3);
};

doctest::String toString(const ObservableChangeNotify& value) {
    return value.toString().toStdString().c_str();
}

TEST_CASE("observable test...") {
    A a;
    a.observableChain();
    FunctionVerification fv1;
    QObject::connect(&a, &QObservableObject::propertyChanged, fv1);
    a.setA(3);
    CHECK_CALLED(fv1);
    FunctionVerification<int, QString, const A *, const QString &> fv2;
    QObject::connect(&a, &A::aChanged, fv2);
    a.emitValueChange();
    CHECK_CALLED_N(fv2, 1);
    int c = fv2.getArg<0>();
    CHECK_EQ(c, 3);
    QString s = fv2.getArg<1>();
    CHECK_EQ(s, "hello");
    const A *pa = fv2.getArg<2>();
    CHECK_NE(pa, nullptr);
    delete pa;
    const QString &s1 = fv2.getArg<3>();
    CHECK_EQ(s1, "world");

    SUBCASE("test pointer property") {
        B_Pointer bp;
        bp.observableChain();

        FunctionVerification fv3;
        QObject::connect(&bp, &QObservableObject::propertyChanged, fv3);

        bp.setPMap(new IntAMap);
        bp.observableChain();

        CHECK_CALLED(fv3);
        A *a1 = new A;
        bp.getPMap()->insert(1, a1);
        bp.observableChain();
        //pointer is not QObservableObject, so it will not be chained
        a1->setA(2);
        CHECK_NOT_CALLED(fv3);

        delete a1;
        delete bp.getPMap();
    }

    SUBCASE("test QPair") {
        B_Pair bp;
        bp.observableChain();
        FunctionVerification fv4;
        QObject::connect(&bp, &QObservableObject::propertyChanged, fv4);
        auto pair = bp.getPair3(); //copyable
        pair.second = 3; //int can't be observable
        CHECK_NOT_CALLED(fv4);
        A *a1 = new A;
        A *a2 = new A;
        QPair<A *, A *> p2;
        p2.first = a1;
        p2.second = a2;
        bp.setPair2(p2);
        CHECK_CALLED(fv4);
        bp.observableChain();
        a1->setA(5);
        CHECK_CALLED(fv4);
        a2->setA(3);
        CHECK_CALLED(fv4);
        delete a1;
        delete a2;
    }

    SUBCASE("test nest") {
        B_List bl;
        FunctionVerification fv5;
        QObject::connect(&bl, &QObservableObject::propertyChanged, fv5);
        auto a1 = new A;
        IntAHash hash = {{1, a1}};
        QList<IntAHash> list = {hash};
        bl.setList(list);
        bl.observableChain();
        a1->setA(3);
        CHECK_CALLED(fv5);
        QObject::connect(a1, &QObservableObject::propertyChanged, fv5);
        a1->setA(5);
        CHECK_CALLED_N(fv5, 2);
        delete a1;
    }

    SUBCASE("test hash") {
        B_Hash bh;
        bh.observableChain();
        FunctionVerification fv6;
        QObject::connect(&bh, &QObservableObject::propertyChanged, fv6);
        bh.setHash(IntAHash{{1, new A}});
        CHECK_CALLED(fv6);
        bh.observableChain();
        bh.getHash().value(1)->setA(3);
        CHECK_CALLED(fv6);
        delete bh.getHash().value(1);
    }

    SUBCASE("test notify") {
        B_Map bm;
        bm.observableChain();
        FunctionVerification<ObservableChangeNotify> fv7;
        QObject::connect(&bm, &QObservableObject::propertyChanged, fv7);
        auto maa = new A;
        bm.setMapA(IntAMap{{1, maa}});
        CHECK_CALLED(fv7);
        const ObservableChangeNotify& notify = fv7.getArg<0>();
        CHECK_FALSE(notify.hasPre());
        CHECK_FALSE(notify.hasNext());
        CHECK(&notify.first() == &notify.last());
        CHECK_EQ(notify.propagatePath(), "B_Map");
        bm.observableChain();
        bm.getMapA().value(1)->setA(3);
        CHECK_CALLED(fv7);
        INFO(notify);
        CHECK_EQ(notify.propertyName(), "a");
        CHECK(IS_PROPERTY(notify.propertyName(), a));
        CHECK_EQ(notify.sourceTypeName(), STATIC_NAME_OF(A));
        CHECK_EQ(notify.propagateSource(), bm.getMapA().value(1));
        CHECK_EQ(notify.propertyTypeName(), "int");
        CHECK_EQ(notify.propertyValue<int>(), 3);
        CHECK_NE(notify.propagatePath(), "A->B_Map");
        CHECK(notify.checkOnPropagatePath<A>());
        CHECK(notify.checkOnPropagatePath<B_Map>());
        CHECK(notify.checkOnPropagatePath<A,B_Map>());
        CHECK_FALSE(notify.checkOnPropagatePath<A, B, B_Map>());
        CHECK_FALSE(notify.checkOnPropagatePath<A, B_Map, B>());
        CHECK(notify.checkOnPropagatePath(maa));
        CHECK(notify.checkOnPropagatePath(&bm));
        CHECK(notify.checkOnPropagatePath(maa, &bm));
        CHECK_FALSE(notify.checkOnPropagatePath(&a));
        CHECK_FALSE(notify.checkOnPropagatePath(maa, &bm, maa));

        delete bm.getMapA().value(1);
    }

    SUBCASE("test propagation") {
        B_List bl;
        FunctionVerification fv8;
        QObject::connect(&bl, &QObservableObject::propertyChanged, fv8);
        auto a1 = new A;
        IntAHash hash = {{1, a1}};
        QList list = {hash};
        bl.setList(list);
        bl.observableChain();
        a1->setPropagation(false);
        a1->setA(3);
        CHECK_NOT_CALLED(fv8);
        a1->setPropagation(true);
        bl.setShot(1);
        a1->setA(4);
        CHECK_CALLED(fv8);
        a1->setA(5);
        CHECK_NOT_CALLED(fv8);
        bl.enableShot();
        a1->setA(6);
        CHECK_CALLED(fv8);
        bl.disableShotOnce();
        a1->setA(7);
        CHECK_NOT_CALLED(fv8);
        a1->setA(8);
        CHECK_CALLED(fv8);
        a1->setA(9);
        CHECK_CALLED(fv8);

        delete a1;
    }

    SUBCASE("test shot guard") {
        B_List bl;
        FunctionVerification fv9;
        QObject::connect(&bl, &QObservableObject::propertyChanged, fv9);
        auto a1 = new A;
        IntAHash hash = {{1, a1}};
        QList list = {hash};
        bl.setList(list);
        bl.observableChain();

        bl.disableShotGuarded();
        a1->setA(2);
        CHECK_CALLED(fv9); //不保持返回值，没有任何效果
        {
            auto g = bl.disableShotGuarded();
            Q_UNUSED(g);
            a1->setA(3);
            a1->setA(4);
            CHECK_NOT_CALLED(fv9);
        }
        a1->setA(5);
        CHECK_CALLED(fv9);
        bl.disableShot();
        a1->setA(6);
        CHECK_NOT_CALLED(fv9);
        {
            auto g = bl.enableShotGuarded();
            a1->setA(7);
            CHECK_CALLED(fv9);
            g.dismiss();
            g.dismiss(); //harmless
            a1->setA(8);
            CHECK_NOT_CALLED(fv9);
        }
        a1->setA(9);
        CHECK_NOT_CALLED(fv9);
        delete a1;
    }

    SUBCASE("test group change") {
        B_List bl;
        FunctionVerification<ObservableChangeNotify> fv;
        QObject::connect(&bl, &QObservableObject::propertyChanged, fv);
        auto a1 = new A;
        auto b1 = new B;
        FunctionVerification vb;
        QObject::connect(b1, &QObservableObject::propertyChanged, vb);
        a1->setPB(b1);
        IntAHash hash = {{1, a1}};
        QList list = {hash};
        bl.setList(list);
        bl.observableChain();

        a1->beginGroupChange();
        CHECK(a1->isGroupChangeBegin());
        list.append(IntAHash{});
        bl.setList(list);
        CHECK_CALLED(fv); //父节点的属性变化正常通知
        b1->setB(2);
        CHECK_CALLED(vb); //子节点自身属性变化正常通知
        CHECK_NOT_CALLED(fv); //但是不会向上传播变化
        a1->endGroupChange();
        CHECK_CALLED(fv);
        CHECK_FALSE(a1->isGroupChangeBegin());
        const ObservableChangeNotify& notify1 = fv.getArg<0>();
        CHECK_EQ(notify1.sourceTypeName(), "B");
        CHECK_EQ(&notify1.first(), &notify1.last()); //只有一个notify

        a1->beginGroupChange();
        WARN_THROWS_AS(a1->beginGroupChange(), std::runtime_error);
        a1->setA(3);
        CHECK_NOT_CALLED(fv); //不会向上传播变化
        a1->setB(3);
        CHECK_NOT_CALLED(fv);
        b1->setB(3);
        CHECK_NOT_CALLED(fv);
        a1->endGroupChange();
        CHECK_CALLED_N(fv, 1); //一组变化只通知一次
        {
            const ObservableChangeNotify& notify2 = fv.getArg<0>(); //这里第一个notify, a1->setA;
            int n = 0;
            for (const ObservableChangeNotify& noti : notify2) {
                n++;
                CHECK(noti.isValid());
            }
            CHECK_EQ(n, 3); //三个属性变化
            CHECK(notify2.checkOnPropagatePath<A, B_List>());
            CHECK(notify2.next().checkOnPropagatePath<A, B_List>());
            CHECK(notify2.next().next().checkOnPropagatePath<B, A, B_List>());
        }

        delete a1;
        delete b1;
    }

    SUBCASE("test validator") {
        A a;
        a.setAValidator([](int v, const QObservableObject *) {
            return v > 10 ;
        });
        auto state = a.setA(3);
        CHECK_EQ(state, false);
        CHECK_EQ(a.getA(), 0);
        a.setA(11);
        CHECK_EQ(a.getA(), 11);
        a.setAValidator([](int v, const QObservableObject *) {
            if (v <= 10) {
                throw std::out_of_range("value must be greater than 10");
            }
            return true;
        });

        CHECK_THROWS_AS(a.setA(3), std::out_of_range);
        CHECK_EQ(a.getA(), 11);

        //clear validator
        a.setAValidator(nullptr);
        a.setA(3);
        CHECK_EQ(a.getA(), 3);


        ValidatorTest t;
        CHECK_THROWS_AS(t.setA(3), XValidationError);
        CHECK_EQ(t.getA(), 0);

        auto error = t.setList({1, 2});
        CHECK_EQ(error, false);

        error = t.setList({1, 2, 3});
        CHECK_EQ(error, true);

        CHECK_FALSE(t.setB(5));

        DeleterTest deleter;

        QList<int> list{1,2,3};
        deleter.setList(list);
        deleter.setList(list);

        CHECK_EQ(deleter.getList(), list);

        int* pav = new int;
        *pav = 3;
        deleter.seta(pav);
        CHECK_EQ(*deleter.geta(), 3);

        int* pav2 = new int;
        *pav2 = 4;
        deleter.seta(pav2);

        CHECK_EQ(*deleter.geta(), 4);

        int* pbv = new int[2];
        pbv[0] = 1;
        pbv[1] = 2;
        deleter.setb(pbv);
        deleter.setb(pbv);//self assigment, can't delete

        CHECK_EQ(deleter.getb()[0], pbv[0]);
        CHECK_EQ(deleter.getb()[1], pbv[1]);

        int wait = 0;
    }

}

#include "observable_test.moc"
