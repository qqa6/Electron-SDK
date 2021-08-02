export enum NativeEngineEvents {
  onJoinChannelSuccess = "onJoinChannelSuccess",
  onLeaveChannel = "onLeaveChannel",
  onUserOffline = "onUserOffline",
  onFirstLocalVideoFrame = "onFirstLocalVideoFrame",
  onFirstRemoteVideoFrame = "onFirstRemoteVideoFrame",
  onAudioVolumeIndication = "onAudioVolumeIndication",
}

export enum NativeVideoSourceEvents {
  onFirstLocalVideoFrame = "onFirstLocalVideoFrame",
  onFirstRemoteVideoDecoded = "onFirstRemoteVideoDecoded",
}
