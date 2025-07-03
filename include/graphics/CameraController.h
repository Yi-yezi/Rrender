#pragma once
#include "core/InputListener.h"
#include <glm/glm.hpp>

namespace graphics {
    /**
     * @brief 相机控制器类，用于处理相机的移动和旋转
     */
    class CameraController : public core::InputListener {
        public:
            /**
             * @brief 相机模式枚举
             */
            enum class CameraMode {
                Arcball,      ///< 轨迹球模式
                FirstPerson   ///< 第一人称模式
            };

            /**
             * @brief 构造函数，初始化相机控制器
             */
            CameraController();

            /**
             * @brief 设置相机模式
             * @param mode 相机模式
             */
            void SetMode(CameraMode mode);

            /**
             * @brief 获取当前相机模式
             * @return 当前相机模式
             */
            CameraMode GetMode() const;

            /**
             * @brief 获取当前视图矩阵
             * @return 视图矩阵
             */
            glm::mat4 GetViewMatrix() const;

            /**
             * @brief 获取相机位置
             * @return 相机位置
             */
            glm::vec3 GetPosition() const;

            /**
             * @brief 键盘输入处理
             */
            void onKey(int key, int scancode, int action, int mods) override;

            /**
             * @brief 鼠标按钮处理
             */
            void onMouseButton(int button, int action, int mods) override;

            /**
             * @brief 鼠标移动处理
             */
            void onMouseMove(double xPos, double yPos, double deltaX, double deltaY) override;

            /**
             * @brief 鼠标滚轮处理
             */
            void onScroll(double xOffset, double yOffset) override;

        private:
            CameraMode m_Mode; ///< 当前相机模式

            // 通用参数
            glm::vec3 m_Position; ///< 相机位置
            glm::vec3 m_Front;    ///< 相机前向
            glm::vec3 m_Up;       ///< 相机上向
            glm::vec3 m_Right;    ///< 相机右向
            glm::vec3 m_WorldUp;  ///< 世界上向

            // 第一人称模式参数
            float m_Yaw;              ///< 偏航角
            float m_Pitch;            ///< 俯仰角
            float m_MoveSpeed;        ///< 移动速度
            float m_MouseSensitivity; ///< 鼠标灵敏度

            // 轨迹球模式参数
            glm::vec3 m_Target;        ///< 目标点
            float m_DistanceToTarget;  ///< 距离目标点的距离
            float m_ArcballYaw;        ///< 轨迹球偏航角
            float m_ArcballPitch;      ///< 轨迹球俯仰角


    };
}

