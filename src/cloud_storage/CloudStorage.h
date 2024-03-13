/**
 * Created March 13, 2024
 *
 * The MIT License (MIT)
 * Copyright (c) 2024 K. Suwatchai (Mobizt)
 *
 *
 * Permission is hereby granted, free of charge, to any person returning a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef ASYNC_CLOUD_STORAGE_H
#define ASYNC_CLOUD_STORAGE_H
#include <Arduino.h>
#include "./core/FirebaseApp.h"

using namespace firebase;

#if defined(ENABLE_CLOUD_STORAGE)

#include "./cloud_storage/DataOptions.h"

class CloudStorage
{
private:
    AsyncClientClass *aClient = nullptr;
    String service_url;
    String path;
    String uid;
    uint32_t app_addr = 0;
    app_token_t *app_token = nullptr;

public:
    ~CloudStorage(){};
    CloudStorage(const String &url = "")
    {
        this->service_url = url;
    };

    CloudStorage &operator=(CloudStorage &rhs)
    {
        this->service_url = rhs.service_url;
        return *this;
    }

    /**
     * Set the Google Cloud Storage URL
     * @param url The Google Cloud Storage URL.
     */
    void url(const String &url)
    {
        this->service_url = url;
    }

    void setApp(uint32_t app_addr, app_token_t *app_token)
    {
        this->app_addr = app_addr;
        this->app_token = app_token;
    }

    app_token_t *appToken()
    {
        List vec;
        return vec.existed(aVec, app_addr) ? app_token : nullptr;
    }

    /**
     * Perform the async task repeatedly.
     * Should be places in main loop function.
     */
    void loop()
    {
        for (size_t i = 0; i < cVec.size(); i++)
        {
            AsyncClientClass *aClient = reinterpret_cast<AsyncClientClass *>(cVec[i]);
            if (aClient)
            {
                aClient->process(true);
                aClient->handleRemove();
            }
        }
    }

    /** Download object from the Google Cloud Storage.
     *
     * @param aClient The async client.
     * @param parent The GoogleCloudStorage::Parent object included Storage bucket Id and object in its constructor.
     * The bucketid is the Storage bucket Id of object to download.
     * The object is the object in Storage bucket to download.
     * @param file The filesystem data (file_config_data) obtained from FileConfig class object.
     * @param options Optional. The GoogleCloudStorage::GetOptions that holds the get options.
     * For the get options, see https://cloud.google.com/storage/docs/json_api/v1/objects/get#optional-parameters
     *
     * @return Boolean value, indicates the success of the operation.
     *
     */
    bool download(AsyncClientClass &aClient, const GoogleCloudStorage::Parent &parent, file_config_data file, GoogleCloudStorage::GetOptions &options)
    {
        AsyncResult result;
        sendRequest(aClient, &result, NULL, "", parent, file, &options, nullptr, nullptr, GoogleCloudStorage::google_cloud_storage_request_type_download, false);
        return result.lastError.code() == 0;
    }

    /** Download object from the Google Cloud Storage.
     *
     * @param aClient The async client.
     * @param parent The GoogleCloudStorage::Parent object included Storage bucket Id and object in its constructor.
     * The bucketid is the Storage bucket Id of object to download.
     * The object is the object in Storage bucket to download.
     * @param file The filesystem data (file_config_data) obtained from FileConfig class object.
     * @param options Optional. The GoogleCloudStorage::GetOptions that holds the get options.
     * For the get options, see https://cloud.google.com/storage/docs/json_api/v1/objects/get#optional-parameters
     * @param aResult The async result (AsyncResult).
     *
     */
    void download(AsyncClientClass &aClient, const GoogleCloudStorage::Parent &parent, file_config_data file, GoogleCloudStorage::GetOptions &options, AsyncResult &aResult)
    {
        sendRequest(aClient, &aResult, NULL, "", parent, file, &options, nullptr, nullptr, GoogleCloudStorage::google_cloud_storage_request_type_download, true);
    }

    /** Download object from the Google Cloud Storage.
     *
     * @param aClient The async client.
     * @param parent The GoogleCloudStorage::Parent object included Storage bucket Id and object in its constructor.
     * The bucketid is the Storage bucket Id of object to download.
     * The object is the object in Storage bucket to download.
     * @param file The filesystem data (file_config_data) obtained from FileConfig class object.
     * @param options Optional. The GoogleCloudStorage::GetOptions that holds the get options.
     * For the get options, see https://cloud.google.com/storage/docs/json_api/v1/objects/get#optional-parameters
     * @param cb The async result callback (AsyncResultCallback).
     * @param uid The user specified UID of async result (optional).
     *
     */
    void download(AsyncClientClass &aClient, const GoogleCloudStorage::Parent &parent, file_config_data file, GoogleCloudStorage::GetOptions &options, AsyncResultCallback cb, const String &uid = "")
    {
        sendRequest(aClient, nullptr, cb, uid, parent, file, &options, nullptr, nullptr, GoogleCloudStorage::google_cloud_storage_request_type_download, true);
    }

    /** Upload file to the Google Cloud Storage.
     *
     * @param aClient The async client.
     * @param parent The GoogleCloudStorage::Parent object included Storage bucket Id and object in its constructor.
     * The bucketid is the Storage bucket Id of object to upload.
     * The object is the object to be stored in the Storage bucket.
     * @param file The filesystem data (file_config_data) obtained from FileConfig class object.
     * @param options Optional. The GoogleCloudStorage::uploadOptions that holds the information for insert options, properties and types of upload.
     * For the insert options (options.insertOptions), see https://cloud.google.com/storage/docs/json_api/v1/objects/insert#optional-parameters
     * For insert properties (options.insertProps), see https://cloud.google.com/storage/docs/json_api/v1/objects/insert#optional-properties
     *
     * @return Boolean value, indicates the success of the operation.
     *
     */
    bool upload(AsyncClientClass &aClient, const GoogleCloudStorage::Parent &parent, file_config_data file, GoogleCloudStorage::uploadOptions &options)
    {
        AsyncResult result;
        sendRequest(aClient, &result, NULL, "", parent, file, nullptr, &options, nullptr, GoogleCloudStorage::google_cloud_storage_request_type_uploads, false);
        return result.lastError.code() == 0;
    }

    /** Upload file to the Google Cloud Storage.
     *
     * @param aClient The async client.
     * @param parent The GoogleCloudStorage::Parent object included Storage bucket Id and object in its constructor.
     * The bucketid is the Storage bucket Id of object to upload.
     * The object is the object to be stored in the Storage bucket.
     * @param file The filesystem data (file_config_data) obtained from FileConfig class object.
     * @param options Optional. The GoogleCloudStorage::uploadOptions that holds the information for insert options, properties and types of upload.
     * For the insert options (options.insertOptions), see https://cloud.google.com/storage/docs/json_api/v1/objects/insert#optional-parameters
     * For insert properties (options.insertProps), see https://cloud.google.com/storage/docs/json_api/v1/objects/insert#optional-properties
     * @param aResult The async result (AsyncResult).
     *
     *
     */
    void upload(AsyncClientClass &aClient, const GoogleCloudStorage::Parent &parent, file_config_data file, GoogleCloudStorage::uploadOptions &options, AsyncResult &aResult)
    {
        sendRequest(aClient, &aResult, NULL, "", parent, file, nullptr, &options, nullptr, GoogleCloudStorage::google_cloud_storage_request_type_uploads, true);
    }

    /** Upload file to the Google Cloud Storage.
     *
     * @param aClient The async client.
     * @param parent The GoogleCloudStorage::Parent object included Storage bucket Id and object in its constructor.
     * The bucketid is the Storage bucket Id of object to upload.
     * The object is the object to be stored in the Storage bucket.
     * @param file The filesystem data (file_config_data) obtained from FileConfig class object.
     * @param options Optional. The GoogleCloudStorage::uploadOptions that holds the information for insert options, properties and types of upload.
     * For the insert options (options.insertOptions), see https://cloud.google.com/storage/docs/json_api/v1/objects/insert#optional-parameters
     * For insert properties (options.insertProps), see https://cloud.google.com/storage/docs/json_api/v1/objects/insert#optional-properties
     * @param cb The async result callback (AsyncResultCallback).
     * @param uid The user specified UID of async result (optional).
     *
     */
    void upload(AsyncClientClass &aClient, const GoogleCloudStorage::Parent &parent, file_config_data file, GoogleCloudStorage::uploadOptions &options, AsyncResultCallback cb, const String &uid = "")
    {
        sendRequest(aClient, nullptr, cb, uid, parent, file, nullptr, &options, nullptr, GoogleCloudStorage::google_cloud_storage_request_type_uploads, true);
    }

    /** Perform OTA update using a firmware (object) from the Google Cloud Storage.
     *
     * @param aClient The async client.
     * @param parent The GoogleCloudStorage::Parent object included Storage bucket Id and object in its constructor.
     * The bucketid is the Storage bucket Id of object to download.
     * The object is the object in Storage bucket to download.
     * @param options Optional. The GoogleCloudStorage::GetOptions that holds the get options.
     * For the get options, see https://cloud.google.com/storage/docs/json_api/v1/objects/get#optional-parameters
     *
     * @return Boolean value, indicates the success of the operation.
     *
     */
    bool ota(AsyncClientClass &aClient, const GoogleCloudStorage::Parent &parent, GoogleCloudStorage::GetOptions &options)
    {
        AsyncResult result;
        file_config_data file;
        sendRequest(aClient, &result, NULL, "", parent, file, &options, nullptr, nullptr, GoogleCloudStorage::google_cloud_storage_request_type_download_ota, false);
        return result.lastError.code() == 0;
    }

    /** Perform OTA update using a firmware (object) from the Google Cloud Storage.
     *
     * @param aClient The async client.
     * @param parent The GoogleCloudStorage::Parent object included Storage bucket Id and object in its constructor.
     * The bucketid is the Storage bucket Id of object to download.
     * The object is the object in Storage bucket to download.
     * @param options Optional. The GoogleCloudStorage::GetOptions that holds the get options.
     * For the get options, see https://cloud.google.com/storage/docs/json_api/v1/objects/get#optional-parameters
     * @param aResult The async result (AsyncResult).
     *
     */
    void ota(AsyncClientClass &aClient, const GoogleCloudStorage::Parent &parent, GoogleCloudStorage::GetOptions &options, AsyncResult &aResult)
    {
        file_config_data file;
        sendRequest(aClient, &aResult, NULL, "", parent, file, &options, nullptr, nullptr, GoogleCloudStorage::google_cloud_storage_request_type_download_ota, true);
    }

    /** Perform OTA update using a firmware (object) from the Google Cloud Storage.
     *
     * @param aClient The async client.
     * @param parent The GoogleCloudStorage::Parent object included Storage bucket Id and object in its constructor.
     * The bucketid is the Storage bucket Id of object to download.
     * The object is the object in Storage bucket to download.
     * @param options Optional. The GoogleCloudStorage::GetOptions that holds the get options.
     * For the get options, see https://cloud.google.com/storage/docs/json_api/v1/objects/get#optional-parameters
     * @param cb The async result callback (AsyncResultCallback).
     * @param uid The user specified UID of async result (optional).
     *
     */
    void ota(AsyncClientClass &aClient, const GoogleCloudStorage::Parent &parent, GoogleCloudStorage::GetOptions &options, AsyncResultCallback cb, const String &uid = "")
    {
        file_config_data file;
        sendRequest(aClient, nullptr, cb, uid, parent, file, &options, nullptr, nullptr, GoogleCloudStorage::google_cloud_storage_request_type_download_ota, true);
    }

    /** Get the metadata of object in Google Cloud Storage data bucket.
     *
     * @param aClient The async client.
     * @param parent The GoogleCloudStorage::Parent object included Storage bucket Id and object in its constructor.
     * The bucketid is the Storage bucket Id of object to get metadata.
     * The object is the object in Storage bucket to get metadata.
     * @param options Optional. The GoogleCloudStorage::GetOptions that holds the get options.
     * For the get options, see https://cloud.google.com/storage/docs/json_api/v1/objects/get#optional-parameters
     *
     * @return Boolean value, indicates the success of the operation.
     *
     */
    bool getMetadata(AsyncClientClass &aClient, const GoogleCloudStorage::Parent &parent, GoogleCloudStorage::GetOptions &options)
    {
        AsyncResult result;
        file_config_data file;
        sendRequest(aClient, &result, NULL, "", parent, file, &options, nullptr, nullptr, GoogleCloudStorage::google_cloud_storage_request_type_get_meta, false);
        return result.lastError.code() == 0;
    }

    /** Get the metadata of object in Google Cloud Storage data bucket.
     *
     * @param aClient The async client.
     * @param parent The GoogleCloudStorage::Parent object included Storage bucket Id and object in its constructor.
     * The bucketid is the Storage bucket Id of object to get metadata.
     * The object is the object in Storage bucket to get metadata.
     * @param options Optional. The GoogleCloudStorage::GetOptions that holds the get options.
     * For the get options, see https://cloud.google.com/storage/docs/json_api/v1/objects/get#optional-parameters
     * @param aResult The async result (AsyncResult).
     *
     */
    void getMetadata(AsyncClientClass &aClient, const GoogleCloudStorage::Parent &parent, GoogleCloudStorage::GetOptions &options, AsyncResult &aResult)
    {
        file_config_data file;
        sendRequest(aClient, &aResult, NULL, "", parent, file, &options, nullptr, nullptr, GoogleCloudStorage::google_cloud_storage_request_type_get_meta, true);
    }

    /** Get the metadata of object in Google Cloud Storage data bucket.
     *
     * @param aClient The async client.
     * @param parent The GoogleCloudStorage::Parent object included Storage bucket Id and object in its constructor.
     * The bucketid is the Storage bucket Id of object to get metadata.
     * The object is the object in Storage bucket to get metadata.
     * @param options Optional. The GoogleCloudStorage::GetOptions that holds the get options.
     * For the get options, see https://cloud.google.com/storage/docs/json_api/v1/objects/get#optional-parameters
     * @param cb The async result callback (AsyncResultCallback).
     * @param uid The user specified UID of async result (optional).
     *
     */
    void getMetadata(AsyncClientClass &aClient, const GoogleCloudStorage::Parent &parent, GoogleCloudStorage::GetOptions &options, AsyncResultCallback cb, const String &uid = "")
    {
        file_config_data file;
        sendRequest(aClient, nullptr, cb, uid, parent, file, &options, nullptr, nullptr, GoogleCloudStorage::google_cloud_storage_request_type_get_meta, true);
    }

    /** List all objects in Google Cloud Storage data bucket.
     *
     * @param aClient The async client.
     * @param parent The GoogleCloudStorage::Parent object included Storage bucket Id in its constructor.
     * The bucketid is the Storage bucket Id to list all objects.
     * @param options Optional. The GoogleCloudStorage::ListOptions that holds the list options.
     * For the list options, see https://cloud.google.com/storage/docs/json_api/v1/objects/list#optional-parameters
     * @return Boolean value, indicates the success of the operation.
     *
     */
    bool list(AsyncClientClass &aClient, const GoogleCloudStorage::Parent &parent, GoogleCloudStorage::ListOptions &options)
    {
        AsyncResult result;
        file_config_data file;
        sendRequest(aClient, &result, NULL, "", parent, file, nullptr, nullptr, &options, GoogleCloudStorage::google_cloud_storage_request_type_list, false);
        return result.lastError.code() == 0;
    }

    /** List all objects in Google Cloud Storage data bucket.
     *
     * @param aClient The async client.
     * @param parent The GoogleCloudStorage::Parent object included Storage bucket Id in its constructor.
     * The bucketid is the Storage bucket Id to list all objects.
     * @param options Optional. The GoogleCloudStorage::ListOptions that holds the list options.
     * For the list options, see https://cloud.google.com/storage/docs/json_api/v1/objects/list#optional-parameters
     * @param aResult The async result (AsyncResult).
     *
     */
    void list(AsyncClientClass &aClient, const GoogleCloudStorage::Parent &parent, GoogleCloudStorage::ListOptions &options, AsyncResult &aResult)
    {
        file_config_data file;
        sendRequest(aClient, &aResult, NULL, "", parent, file, nullptr, nullptr, &options, GoogleCloudStorage::google_cloud_storage_request_type_list, true);
    }

    /** List all objects in Google Cloud Storage data bucket.
     *
     * @param aClient The async client.
     * @param parent The GoogleCloudStorage::Parent object included Storage bucket Id in its constructor.
     * The bucketid is the Storage bucket Id to list all objects.
     * @param options Optional. The GoogleCloudStorage::ListOptions that holds the list options.
     * For the list options, see https://cloud.google.com/storage/docs/json_api/v1/objects/list#optional-parameters
     * @param cb The async result callback (AsyncResultCallback).
     * @param uid The user specified UID of async result (optional).
     *
     */
    void list(AsyncClientClass &aClient, const GoogleCloudStorage::Parent &parent, GoogleCloudStorage::ListOptions &options, AsyncResultCallback cb, const String &uid = "")
    {
        file_config_data file;
        sendRequest(aClient, nullptr, cb, uid, parent, file, nullptr, nullptr, &options, GoogleCloudStorage::google_cloud_storage_request_type_list, true);
    }

    /** Delete the object in Google Cloud Storage data bucket.
     *
     * @param aClient The async client.
     * @param parent The GoogleCloudStorage::Parent object included Storage bucket Id and object in its constructor.
     * The bucketid is the Storage bucket Id of object to delete.
     * The object is the object in Storage bucket to delete.
     * @param options Optional. The GoogleCloudStorage::DeleteOptions that holds the delete options.
     * For the delete options, see see https://cloud.google.com/storage/docs/json_api/v1/objects/delete#optional-parameters
     *
     * @return Boolean value, indicates the success of the operation.
     *
     */
    bool deleteObject(AsyncClientClass &aClient, const GoogleCloudStorage::Parent &parent, GoogleCloudStorage::DeleteOptions options)
    {
        AsyncResult result;
        file_config_data file;
        sendRequest(aClient, &result, NULL, "", parent, file, &options, nullptr, nullptr, GoogleCloudStorage::google_cloud_storage_request_type_delete, false);
        return result.lastError.code() == 0;
    }

    /** Delete the object in Google Cloud Storage data bucket.
     *
     * @param aClient The async client.
     * @param parent The GoogleCloudStorage::Parent object included Storage bucket Id and object in its constructor.
     * The bucketid is the Storage bucket Id of object to delete.
     * The object is the object in Storage bucket to delete.
     * @param options Optional. The GoogleCloudStorage::DeleteOptions that holds the delete options.
     * For the delete options, see see https://cloud.google.com/storage/docs/json_api/v1/objects/delete#optional-parameters
     * @param aResult The async result (AsyncResult).
     *
     */
    void deleteObject(AsyncClientClass &aClient, const GoogleCloudStorage::Parent &parent, GoogleCloudStorage::DeleteOptions options, AsyncResult &aResult)
    {
        file_config_data file;
        sendRequest(aClient, &aResult, NULL, "", parent, file, &options, nullptr, nullptr, GoogleCloudStorage::google_cloud_storage_request_type_delete, true);
    }

    /** Delete the object in Firebase Storage data bucket.
     *
     * @param aClient The async client.
     * @param parent The GoogleCloudStorage::Parent object included Storage bucket Id and object in its constructor.
     * The bucketid is the Storage bucket Id of object to delete.
     * The object is the object in Storage bucket to delete.
     * @param options Optional. The GoogleCloudStorage::DeleteOptions that holds the delete options.
     * For the delete options, see see https://cloud.google.com/storage/docs/json_api/v1/objects/delete#optional-parameters
     * @param cb The async result callback (AsyncResultCallback).
     * @param uid The user specified UID of async result (optional).
     *
     */
    void deleteObject(AsyncClientClass &aClient, const GoogleCloudStorage::Parent &parent, GoogleCloudStorage::DeleteOptions options, AsyncResultCallback cb, const String &uid = "")
    {
        file_config_data file;
        sendRequest(aClient, nullptr, cb, uid, parent, file, &options, nullptr, nullptr, GoogleCloudStorage::google_cloud_storage_request_type_delete, true);
    }

    void sendRequest(AsyncClientClass &aClient, AsyncResult *result, AsyncResultCallback cb, const String &uid, const GoogleCloudStorage::Parent &parent, file_config_data &file, GoogleCloudStorage::BaseOptions *baseOptions, GoogleCloudStorage::uploadOptions *uploadOptions, GoogleCloudStorage::ListOptions *listOptions, GoogleCloudStorage::google_cloud_storage_request_type requestType, bool async)
    {
        GoogleCloudStorage::DataOptions options;
        options.requestType = requestType;
        options.parent = parent;
        async_request_handler_t::http_request_method method = async_request_handler_t::http_post;
    }

    void asyncRequest(GoogleCloudStorage::async_request_data_t &request, int beta = 0)
    {
        app_token_t *app_token = appToken();

        if (!app_token)
            return setClientError(request, FIREBASE_ERROR_APP_WAS_NOT_ASSIGNED);

        request.opt.app_token = app_token;
        String extras;

        addParams(request, extras);

        url(FPSTR("cloudstorage.googleapis.com"));

        async_data_item_t *sData = request.aClient->createSlot(request.opt);

        if (!sData)
            return setClientError(request, FIREBASE_ERROR_OPERATION_CANCELLED);

        request.aClient->newRequest(sData, service_url, request.path, extras, request.method, request.opt, request.uid);

        if (request.file)
        {
            sData->request.file_data.copy(*request.file);
            sData->request.base64 = false;
            if (request.mime.length())
                request.aClient->setContentType(sData, request.mime);
            request.aClient->setFileContentLength(sData);
        }
        else if (request.options->payload.length())
        {
            sData->request.val[req_hndlr_ns::payload] = request.options->payload;
            request.aClient->setContentLength(sData, request.options->payload.length());
        }

        setFileStatus(sData, request);

        if (request.opt.ota)
        {
            sData->request.ota = true;
            sData->request.base64 = false;
            sData->aResult.download_data.ota = true;
        }

        if (request.cb)
            sData->cb = request.cb;

        if (request.aResult)
            sData->setRefResult(request.aResult);

        request.aClient->process(sData->async);
        request.aClient->handleRemove();
    }

    void setClientError(GoogleCloudStorage::async_request_data_t &request, int code)
    {
        AsyncResult *aResult = request.aResult;

        if (!aResult)
            aResult = new AsyncResult();

        aResult->error_available = true;
        aResult->lastError.setClientError(code);

        if (request.cb)
            request.cb(*aResult);

        if (!request.aResult)
        {
            delete aResult;
            aResult = nullptr;
        }
    }

    void addParams(GoogleCloudStorage::async_request_data_t &request, String &extras)
    {
        extras += request.options->extras;
        extras.replace(" ", "%20");
        extras.replace(",", "%2C");
    }

    void setFileStatus(async_data_item_t *sData, GoogleCloudStorage::async_request_data_t &request)
    {
        if (sData->request.file_data.filename.length() || request.opt.ota)
        {
            sData->download = request.method == async_request_handler_t::http_get;
            sData->upload = request.method == async_request_handler_t::http_post ||
                            request.method == async_request_handler_t::http_put ||
                            request.method == async_request_handler_t::http_patch;
        }
    }
};

#endif

#endif