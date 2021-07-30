/*
 * @Author: zhangtao@agora.io
 * @Date: 2021-04-28 13:34:31
 * @Last Modified by: zhangtao@agora.io
 * @Last Modified time: 2021-05-10 11:16:41
 */

export const TAG = "[Agora]: ";

export const deprecate = (originApi?: string, replaceApi?: string) => {
  console.warn(
    `${TAG} This method ${originApi} will be deprecated soon. `,
    replaceApi ? `Please use ${replaceApi} instead` : ""
  );
};

export const logWarn = (msg: string, tag: string = TAG) => {
  console.warn(`${tag} ${msg}`);
};

export const logError = (msg: string, tag: string = TAG) => {
  console.error(`${tag} ${msg}`);
};

export const logInfo = (msg: string, tag: string = TAG) => {
  console.log(`${tag} ${msg}`);
};

export const objsKeysToLowerCase = (array: Array<any>) => {
  array.forEach((obj) => {
    for (const key in obj) {
      if (Object.prototype.hasOwnProperty.call(obj, key)) {
        const element = obj[key];
        obj[key.toLocaleLowerCase()] = element;
      }
    }
  });
};

export const changeEventNameForEngine = (eventName: string) =>
  eventName.slice(2, 3).toLocaleLowerCase() + eventName.slice(3);

export const changeEventNameForVideoSource = (eventName: string) =>
  "videoSource" + eventName.slice(2);

export const jsonStringToArray = (jsonString: string) =>
  jsonStringConvertWithDefaultValue(jsonString, []);

export const jsonStringToObj = (jsonString: string) =>
  jsonStringConvertWithDefaultValue(jsonString, {});

export const jsonStringConvertWithDefaultValue = (
  jsonString: string,
  defaultValue: any
) => {
  try {
    return JSON.parse(jsonString);
  } catch (error) {
    logError(`jsonStringConvertWithDefaultValue error: ${jsonString}`);
    return defaultValue;
  }
};
