// GameConfig v11 - Fixed visuals config
export const GameConfig = {
    version: 11,
    
    physics: {
        gravity: -18,
    },
    
    camera: {
        fov: 45,
        near: 0.1,
        far: 1000,
        position: { x: 8, y: 12, z: 8 },
        lookAt: { x: 0, y: 0, z: 0 },
        followSpeed: 0.1
    },

    player: {
        size: { width: 0.5, height: 0.8, depth: 0.5 },
        color: 0x333333,
        startPosition: { x: 0, y: 0.65, z: 0 },
        jump: {
            minPower: 1,
            maxPower: 4,
            chargeRate: 2.5,
            horizontalSpeed: 1.4
        },
        squash: {
            maxSquash: 0.5,
            stretchFactor: 1.4
        }
    },

    platform: {
        size: { width: 1.0, height: 0.5, depth: 1.0 },
        colors: [0x67c23a, 0x409eff, 0xe6a23c, 0xf56c6c, 0x909399],
        distance: {
            min: 1.8,
            max: 3.5
        },
        spawnAnimation: {
            duration: 0.4,
            bounceHeight: 0.15
        },
        maxPlatforms: 8
    },

    scoring: {
        centerZone: 0.15,
        centerBaseScore: 2,
        edgeScore: 1
    },

    visuals: {
        backgroundColor: 0xd4e4ed,
        ambientLightColor: 0xffffff,
        ambientLightIntensity: 0.6,
        directionalLightColor: 0xffffff,
        directionalLightIntensity: 0.8,
        directionalLightPosition: { x: 5, y: 15, z: 5 },
        shadowMapSize: 2048
    },

    animation: {
        cameraFollowSpeed: 0.1
    }
};

console.log(`GameConfig v${GameConfig.version}`);
