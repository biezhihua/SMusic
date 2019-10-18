package com.bzh.splayer.lib

import android.media.MediaCodecList


object Utils {

    private val codecMap = hashMapOf<String, String>()

    init {
        codecMap["h264"] = "video/avc"
    }

    fun findVideoCodecName(codecName: String): String? = codecMap[codecName]

    fun isSupportCodec(mimeType: String): Boolean {
        // 获取所有支持编解码器数量
        val numCodecs = MediaCodecList.getCodecCount()
        for (i in 0..numCodecs) {
            // 编解码器相关性信息存储在MediaCodecInfo中
            val codecInfo = MediaCodecList.getCodecInfoAt(i)
            // 判断是否为编码器
            if (!codecInfo.isEncoder) {
                continue
            }
            // 获取编码器支持的MIME类型，并进行匹配
            val types = codecInfo.supportedTypes
            for (j in types.indices) {
                if (types[j].equals(findVideoCodecName(mimeType), ignoreCase = true)) {
                    return true
                }
            }
        }
        return false
    }


}