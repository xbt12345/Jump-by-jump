// AnimationManager - Handles tweening animations
import { Easing } from './Easing.js';

export class AnimationManager {
    constructor() {
        this.animations = [];
    }

    animate(options) {
        const {
            target,
            from,
            to,
            duration,
            easing = 'linear',
            onUpdate,
            onComplete
        } = options;

        return new Promise(resolve => {
            const animation = {
                target,
                from: { ...from },
                to: { ...to },
                duration,
                elapsed: 0,
                easingFn: typeof easing === 'function' ? easing : Easing[easing] || Easing.linear,
                onUpdate,
                onComplete: () => {
                    if (onComplete) onComplete();
                    resolve();
                },
                isComplete: false
            };
            this.animations.push(animation);
        });
    }

    update(deltaTime) {
        for (let i = this.animations.length - 1; i >= 0; i--) {
            const anim = this.animations[i];
            anim.elapsed += deltaTime;
            
            const progress = Math.min(anim.elapsed / anim.duration, 1);
            const easedProgress = anim.easingFn(progress);
            
            // Interpolate values
            const current = {};
            for (const key in anim.from) {
                current[key] = anim.from[key] + (anim.to[key] - anim.from[key]) * easedProgress;
            }
            
            // Apply to target
            if (anim.target) {
                for (const key in current) {
                    anim.target[key] = current[key];
                }
            }
            
            if (anim.onUpdate) {
                anim.onUpdate(current, easedProgress);
            }
            
            // Check completion
            if (progress >= 1) {
                anim.isComplete = true;
                if (anim.onComplete) anim.onComplete();
                this.animations.splice(i, 1);
            }
        }
    }

    cancelFor(target) {
        this.animations = this.animations.filter(a => a.target !== target);
    }

    clear() {
        this.animations = [];
    }
}
