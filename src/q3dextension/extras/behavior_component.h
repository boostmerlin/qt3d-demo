//
// Created by merlin
//

#ifndef PMAXWELL_QT_BEHAVIOR_COMPONENT_H
#define PMAXWELL_QT_BEHAVIOR_COMPONENT_H

#include <Qt3DCore/QComponent>

//todo: enhance using q3d ecs
class BehaviorComponent : public Qt3DCore::QComponent {
    Q_OBJECT

public:
    explicit BehaviorComponent(Qt3DCore::QNode *parent = nullptr);

    void setOrder(int order);
    int order() const;

    void setValid();
    void setInvalid();
protected:
    //要幂等性
    virtual void onEnable();

    virtual void onDisable();
    //todo:
    // virtual void End();
    // virtual void Start();

private:
    int m_order;
};


#endif //PMAXWELL_QT_BEHAVIOR_COMPONENT_H
