//
// Created by ftfnunes on 19/05/18.
//

#include <Sprite.h>
#include <Collider.h>
#include <LineSegment.h>
#include <Game.h>
#include <Collidable.h>
#include "BeamSkill.h"

BeamSkill::BeamSkill(GameObject &associated, Vec2 target) : Component(associated), target(target), lockBeam(false) {
    auto raySprite = new Sprite(associated, "img/magic_effect_side2.png", 4, BEAM_LIFETIME/4);
    associated.AddComponent(raySprite);
    auto collider = new Collider(associated);
    associated.AddComponent(collider);


    associated.box.y -= associated.box.h/2;
}

BeamSkill::~BeamSkill() {
}

void BeamSkill::Update(float dt) {
    lifeTimer.Update(dt);
    if (lifeTimer.Get() > BEAM_LIFETIME) {
        lifeTimer.Restart();
        initObject.lock()->RequestDelete();
        endObject.lock()->RequestDelete();
        associated.RequestDelete();
    }
}

void BeamSkill::Render() {
    lockBeam = true;
}

bool BeamSkill::Is(string type) {
    return type == BEAM_SKILL_TYPE;
}

void BeamSkill::NotifyCollision(GameObject &other) {
    if (!lockBeam) {
        auto collidable = (Collidable *) other.GetComponent(COLLIDABLE_TYPE);
        auto thisCollider = (Collider *) associated.GetComponent(COLLIDER_TYPE);
        auto colliderBox = thisCollider->box;

        auto intersections = collidable->GetIntersections(*thisCollider);
        auto boxCorners = colliderBox.GetCorners(associated.angleDeg, associated.rotationCenter);
        auto l1 = LineSegment(boxCorners[0], boxCorners[1]);
        auto l2 = LineSegment(boxCorners[2], boxCorners[3]);
        for (auto &intersection : intersections) {
            auto intersectionDot = intersection.second;
            auto intersectionLine = intersection.first;

            if (intersectionLine == l1) {
                auto d = (intersectionDot - l1.dot1).Module();
                if (d < cutoffPoint) {
                    cutoffPoint = d;
                }
            } else if (intersectionLine == l2) {
                auto d = (intersectionDot - l2.dot2).Module();
                if (d < cutoffPoint) {
                    cutoffPoint = d;
                }
            }
        }

        this->associated.box.w = cutoffPoint;
        thisCollider->box.w = cutoffPoint;
        auto sprite = (Sprite *) associated.GetComponent(SPRITE_TYPE);
        auto clip = sprite->GetClip();
        sprite->SetClip(clip.x, clip.y, cutoffPoint, clip.h);

        endObject.lock()->SetCenter((Vec2(associated.box.x + cutoffPoint, associated.box.y + associated.box.h/2) - Vec2(associated.box.x, associated.box.y + associated.box.h/2)).RotateDeg(associated.angleDeg) + Vec2(associated.box.x, associated.box.y + associated.box.h/2));
    }
}

void BeamSkill::Start() {
    auto collider = (Collider *) associated.GetComponent(COLLIDER_TYPE);
    auto sprite = (Sprite *) associated.GetComponent(SPRITE_TYPE);

    auto d = target - Vec2(associated.box.x, associated.box.y + associated.box.h/2);
    associated.rotationCenter = Vec2(0, associated.box.h/2);
    associated.angleDeg = d.XAngleDeg();
    cutoffPoint = d.Module();
    collider->box.w = cutoffPoint;
    associated.box.w = cutoffPoint;
    auto clip = sprite->GetClip();
    sprite->SetClip(clip.x, clip.y, cutoffPoint, clip.h);

    auto initObj = new GameObject(associated.GetLayer());
    initObj->angleDeg = associated.angleDeg;
    initObj->AddComponent(new Sprite(*initObj, "img/magic_effect_side1.png", 4, BEAM_LIFETIME/4));
    initObj->SetCenter(Vec2(associated.box.x, associated.box.y + associated.box.h/2));
    initObject = Game::GetInstance().GetCurrentState().AddObject(initObj);

    auto endObj = new GameObject(associated.GetLayer());
    endObj->angleDeg = associated.angleDeg;
    endObj->AddComponent(new Sprite(*endObj, "img/magic_effect_side3.png", 4, BEAM_LIFETIME/4));
    endObj->SetCenter(target);
    endObject = Game::GetInstance().GetCurrentState().AddObject(endObj);
}