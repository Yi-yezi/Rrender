#pragma once

namespace utils {

    /**
     * @brief 时间管理工具类，提供 deltaTime、当前时间等功能
     * 模拟 Unity 中的 Time.deltaTime, Time.time 等
     */
    class Time {
    public:
        /**
         * @brief 每帧更新时间，需在主循环每帧调用一次
         * @param currentTime 当前时间（一般为 glfwGetTime()）
         */
        static void Update(double currentTime);

        /**
         * @brief 获取当前帧与上一帧的时间间隔（秒）
         */
        static float GetDeltaTime();

        /**
         * @brief 获取程序运行至今的时间（秒）
         */
        static float GetTime();

    private:
        inline static double s_CurrentTime = 0.0;
        inline static double s_LastTime = 0.0;
        inline static double s_DeltaTime = 0.0;
    };

}
