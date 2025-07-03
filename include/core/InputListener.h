#pragma once

namespace core {

    /**
     * @brief 输入监听器接口（观察者）
     * 所有需要响应输入的组件（如 CameraController）继承此类
     */
    class InputListener {
    public:
        virtual ~InputListener() = default;

        /**
         * @brief 每帧被 InputManager 调用
         * 监听者可自行轮询 IsKeyDown、GetMouseDelta 等状态进行响应
         */
        virtual void OnInput() = 0;
    };

}
