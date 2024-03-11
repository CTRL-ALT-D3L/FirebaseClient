/**
 * Created March 11, 2024
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
#ifndef ASYNC_FIRESTORE_BASE_H
#define ASYNC_FIRESTORE_BASE_H
#include <Arduino.h>
#include "./core/FirebaseApp.h"
#include "./firestore/DataOptions.h"

#if defined(ENABLE_FIRESTORE)

using namespace firebase;

#include "./firestore/Query.h"

class FirestoreBase
{
    friend class FirebaseApp;

private:
    String service_url;
    String path;
    String uid;
    uint32_t app_addr = 0;
    app_token_t *app_token = nullptr;

    struct async_request_data_t
    {
    public:
        AsyncClientClass *aClient = nullptr;
        String path;
        String uid;
        async_request_handler_t::http_request_method method = async_request_handler_t::http_undefined;
        slot_options_t opt;
        FirestoreOptions *options = nullptr;
        AsyncResult *aResult = nullptr;
        AsyncResultCallback cb = NULL;
        async_request_data_t() {}
        async_request_data_t(AsyncClientClass *aClient, const String &path, async_request_handler_t::http_request_method method, slot_options_t opt, FirestoreOptions *options, AsyncResult *aResult, AsyncResultCallback cb, const String &uid = "")
        {
            this->aClient = aClient;
            this->path = path;
            this->method = method;
            this->opt = opt;
            this->options = options;
            this->aResult = aResult;
            this->cb = cb;
            this->uid = uid;
        }
    };

public:
    ~FirestoreBase(){};

    FirestoreBase(const String &url = "")
    {
        this->service_url = url;
    };

    FirestoreBase &operator=(FirestoreBase &rhs)
    {
        this->service_url = rhs.service_url;
        return *this;
    }

    /**
     * Set the Firestore URL
     * @param url The Firestore URL.
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

    void asyncRequest(async_request_data_t &request, int beta = 0)
    {
        URLHelper uh;
        app_token_t *app_token = appToken();

        if (!app_token)
            return setClientError(request, FIREBASE_ERROR_APP_WAS_NOT_ASSIGNED);

        request.opt.app_token = app_token;
        String extras;
        if (beta == 2)
            uh.addGAPIv1beta2Path(request.path);
        else if (beta == 1)
            uh.addGAPIv1beta1Path(request.path);
        else
            uh.addGAPIv1Path(request.path);
        request.path += request.options->parent.projectId.length() == 0 ? app_token->project_id : request.options->parent.projectId;
        request.path += FPSTR("/databases");
        if (!request.options->parent.isDatabaseIdParam())
        {
            request.path += '/';
            request.path += request.options->parent.databaseId.length() > 0 ? request.options->parent.databaseId : FPSTR("(default)");
        }
        addParams(request, extras);

        url(FPSTR("firestore.googleapis.com"));

        async_data_item_t *sData = request.aClient->createSlot(request.opt);

        if (!sData)
            return setClientError(request, FIREBASE_ERROR_OPERATION_CANCELLED);

        request.aClient->newRequest(sData, service_url, request.path, extras, request.method, request.opt, request.uid);

        if (request.options->payload.length())
        {
            sData->request.payload = request.options->payload;
            request.aClient->setContentLength(sData, request.options->payload.length());
        }

        if (request.cb)
            sData->cb = request.cb;

        if (request.aResult)
            sData->setRefResult(request.aResult);

        request.aClient->process(sData->async);
        request.aClient->handleRemove();
    }

    void addParams(async_request_data_t &request, String &extras)
    {
        extras += request.options->extras;
        extras.replace(" ", "%20");
        extras.replace(",", "%2C");
    }

    void setClientError(async_request_data_t &request, int code)
    {
        AsyncResult *aResult = request.aResult;

        if (!aResult)
            aResult = new AsyncResult();

        aResult->error_available = true;
        aResult->lastError.setClientError(code);

        if (request.cb)
            request.cb(*aResult);

        if (!request.aResult)
            delete aResult;
    }

    void eximDocs(AsyncClientClass &aClient, AsyncResult *result, AsyncResultCallback cb, const String &uid, const ParentResource &parent, EximDocumentOptions &eximOptions, bool isImport, bool async)
    {
        FirestoreOptions options;
        options.requestType = isImport ? firebase_firestore_request_type_import_docs : firebase_firestore_request_type_export_docs;
        options.parent = parent;
        options.payload = eximOptions.c_str();
        if (!isImport)
            options.payload.replace("inputUriPrefix", "outputUriPrefix");
        options.extras += isImport ? FPSTR(":importDocuments") : FPSTR(":exportDocuments");
        async_request_data_t aReq(&aClient, path, async_request_handler_t::http_post, slot_options_t(false, false, async, false, false, false), &options, result, cb, uid);
        asyncRequest(aReq);
    }

    void manageDatabase(AsyncClientClass &aClient, AsyncResult *result, AsyncResultCallback cb, const String &uid, const ParentResource &parent, const String &database, const String &key, Firestore::firestore_database_mode mode, bool async)
    {
        FirestoreOptions options;
        options.requestType = firebase_firestore_request_type_manage_database;
        options.parent = parent;

        if (strlen(database.c_str()))
        {
            options.payload = database.c_str();
            if (mode == Firestore::firestore_database_mode_create)
            {
                options.parent.setDatabaseIdParam(true);
                options.extras += FPSTR("?databaseId=");
                options.extras += options.parent.databaseId;
            }
        }

        if (key.length())
        {
            if (mode == Firestore::firestore_database_mode_delete)
                options.extras += FPSTR("?etag=");
            else if (mode == Firestore::firestore_database_mode_patch)
                options.extras += FPSTR("?updateMask=");
            options.extras += key;
        }

        if (mode == Firestore::firestore_database_mode_list)
            options.parent.setDatabaseIdParam(true);

        async_request_handler_t::http_request_method method = async_request_handler_t::http_undefined;

        if (strlen(database.c_str()) > 0 && mode == Firestore::firestore_database_mode_create)
            method = async_request_handler_t::http_post; // create
        else if (options.parent.databaseId.length() > 0 && (mode == Firestore::firestore_database_mode_delete || mode == Firestore::firestore_database_mode_get))
            method = mode == Firestore::firestore_database_mode_delete ? async_request_handler_t::http_delete : async_request_handler_t::http_get; // get index or delete by id
        else if (strlen(database.c_str()) == 0 && mode == Firestore::firestore_database_mode_list)
            method = async_request_handler_t::http_get; // list
        else if (strlen(database.c_str()) > 0 && mode == Firestore::firestore_database_mode_patch)
            method = async_request_handler_t::http_patch; // patch

        async_request_data_t aReq(&aClient, path, method, slot_options_t(false, false, async, false, false, false), &options, result, cb, uid);
        asyncRequest(aReq);
    }

    void createDoc(AsyncClientClass &aClient, AsyncResult *result, AsyncResultCallback cb, const String &uid, const ParentResource &parent, const String &collectionId, const String &documentId, DocumentMask &mask, Document<Values::Value> &document, bool async)
    {
        FirestoreOptions options;
        options.requestType = firebase_firestore_request_type_create_doc;
        options.parent = parent;
        options.collectionId = collectionId;
        options.documentId = documentId;
        options.payload = document.c_str();
        addDocsPath(options.extras);
        options.extras += '/';
        options.extras += options.collectionId;
        URLHelper uh;
        bool hasQueryParams = false;
        uh.addParam(options.extras, "documentId", options.documentId, hasQueryParams);
        options.extras += mask.getQuery("mask", hasQueryParams);
        async_request_data_t aReq(&aClient, path, async_request_handler_t::http_post, slot_options_t(false, false, async, false, false, false), &options, result, cb, uid);
        asyncRequest(aReq);
    }

    void patchDoc(AsyncClientClass &aClient, AsyncResult *result, AsyncResultCallback cb, const String &uid, const ParentResource &parent, const String &documentPath, patchDocumentOptions patchOptions, Document<Values::Value> &document, bool async)
    {
        FirestoreOptions options;
        options.requestType = firebase_firestore_request_type_patch_doc;
        options.parent = parent;
        options.payload = document.c_str();
        addDocsPath(options.extras);
        options.extras += '/';
        options.extras += documentPath;
        options.extras += patchOptions.c_str();
        async_request_data_t aReq(&aClient, path, async_request_handler_t::http_patch, slot_options_t(false, false, async, false, false, false), &options, result, cb, uid);
        asyncRequest(aReq);
    }

    void commitDoc(AsyncClientClass &aClient, AsyncResult *result, AsyncResultCallback cb, const String &uid, const ParentResource &parent, Writes &writes, bool async)
    {
        FirestoreOptions options;
        options.requestType = firebase_firestore_request_type_commit_document;
        options.parent = parent;
        options.payload = writes.c_str();
        addDocsPath(options.extras);
        options.extras += FPSTR(":commit");
        options.payload.replace((const char *)RESOURCE_PATH_BASE, makeResourcePath(parent));
        async_request_data_t aReq(&aClient, path, async_request_handler_t::http_post, slot_options_t(false, false, async, false, false, false), &options, result, cb, uid);
        asyncRequest(aReq);
    }

    void batchWriteDoc(AsyncClientClass &aClient, AsyncResult *result, AsyncResultCallback cb, const String &uid, const ParentResource &parent, Writes &writes, bool async)
    {
        FirestoreOptions options;
        options.requestType = firebase_firestore_request_type_batch_write_doc;
        options.parent = parent;
        options.payload = writes.c_str();
        options.payload.replace((const char *)RESOURCE_PATH_BASE, makeResourcePath(parent));
        addDocsPath(options.extras);
        options.extras += FPSTR(":batchWrite");
        async_request_data_t aReq(&aClient, path, async_request_handler_t::http_post, slot_options_t(false, false, async, false, false, false), &options, result, cb, uid);
        asyncRequest(aReq);
    }

    void getDoc(AsyncClientClass &aClient, AsyncResult *result, AsyncResultCallback cb, const String &uid, const ParentResource &parent, const String &documentPath, GetDocumentOptions getOptions, bool async)
    {
        FirestoreOptions options;
        options.requestType = firebase_firestore_request_type_get_doc;
        options.parent = parent;
        addDocsPath(options.extras);
        options.extras += '/';
        options.extras += documentPath;
        options.extras += getOptions.c_str();
        async_request_data_t aReq(&aClient, path, async_request_handler_t::http_get, slot_options_t(false, false, async, false, false, false), &options, result, cb, uid);
        asyncRequest(aReq);
    }

    void batchGetDoc(AsyncClientClass &aClient, AsyncResult *result, AsyncResultCallback cb, const String &uid, const ParentResource &parent, BatchGetDocumentOptions batchOptions, bool async)
    {
        FirestoreOptions options;
        options.requestType = firebase_firestore_request_type_batch_get_doc;
        options.parent = parent;
        options.payload = batchOptions.c_str();
        options.payload.replace((const char *)RESOURCE_PATH_BASE, makeResourcePath(parent));
        addDocsPath(options.extras);
        options.extras += FPSTR(":batchGet");
        async_request_data_t aReq(&aClient, path, async_request_handler_t::http_post, slot_options_t(false, false, async, false, false, false), &options, result, cb, uid);
        asyncRequest(aReq);
    }

    void beginTrans(AsyncClientClass &aClient, AsyncResult *result, AsyncResultCallback cb, const String &uid, const ParentResource &parent, TransactionOptions transOptions, bool async)
    {
        FirestoreOptions options;
        options.requestType = firebase_firestore_request_type_begin_transaction;
        options.parent = parent;
        JsonHelper jh;
        jh.addObject(options.payload, "options", transOptions.c_str(), false, true);
        addDocsPath(options.extras);
        options.extras += FPSTR(":beginTransaction");
        async_request_data_t aReq(&aClient, path, async_request_handler_t::http_post, slot_options_t(false, false, async, false, false, false), &options, result, cb, uid);
        asyncRequest(aReq);
    }

    void transRollback(AsyncClientClass &aClient, AsyncResult *result, AsyncResultCallback cb, const String &uid, const ParentResource &parent, const String &transaction, bool async)
    {
        FirestoreOptions options;
        options.requestType = firebase_firestore_request_type_rollback;
        options.parent = parent;
        JsonHelper jh;
        jh.addObject(options.payload, "transaction", transaction, true, true);
        addDocsPath(options.extras);
        options.extras += FPSTR(":rollback");
        async_request_data_t aReq(&aClient, path, async_request_handler_t::http_post, slot_options_t(false, false, async, false, false, false), &options, result, cb, uid);
        asyncRequest(aReq);
    }

#if defined(ENABLE_FIRESTORE_QUERY)
    void runQueryImpl(AsyncClientClass &aClient, AsyncResult *result, AsyncResultCallback cb, const String &uid, const ParentResource &parent, const String &documentPath, QueryOptions queryOptions, bool async)
    {
        FirestoreOptions options;
        options.requestType = firebase_firestore_request_type_run_query;
        options.parent = parent;
        options.parent.documentPath = documentPath;
        options.payload = queryOptions.c_str();

        addDocsPath(options.extras);
        URLHelper uh;
        uh.addPath(options.extras, documentPath);
        options.extras += FPSTR(":runQuery");
        async_request_data_t aReq(&aClient, path, async_request_handler_t::http_post, slot_options_t(false, false, async, false, false, false), &options, result, cb, uid);
        asyncRequest(aReq);
    }
#endif

    void deleteDocBase(AsyncClientClass &aClient, AsyncResult *result, AsyncResultCallback cb, const String &uid, const ParentResource &parent, const String &documentPath, Precondition currentDocument, bool async)
    {
        FirestoreOptions options;
        options.requestType = firebase_firestore_request_type_delete_doc;
        options.parent = parent;
        options.parent.documentPath = documentPath;

        addDocsPath(options.extras);
        URLHelper uh;
        uh.addPath(options.extras, documentPath);
        options.extras += currentDocument.getQuery("currentDocument");
        async_request_data_t aReq(&aClient, path, async_request_handler_t::http_delete, slot_options_t(false, false, async, false, false, false), &options, result, cb, uid);
        asyncRequest(aReq);
    }

    void listDocs(AsyncClientClass &aClient, AsyncResult *result, AsyncResultCallback cb, const String &uid, const ParentResource &parent, const String &collectionId, ListDocumentsOptions listDocsOptions, bool async)
    {
        FirestoreOptions options;
        options.requestType = firebase_firestore_request_type_list_doc;
        options.parent = parent;

        addDocsPath(options.extras);
        URLHelper uh;
        uh.addPath(options.extras, collectionId);

        options.extras += listDocsOptions.c_str();
        async_request_data_t aReq(&aClient, path, async_request_handler_t::http_get, slot_options_t(false, false, async, false, false, false), &options, result, cb, uid);
        asyncRequest(aReq);
    }

    void listCollIds(AsyncClientClass &aClient, AsyncResult *result, AsyncResultCallback cb, const String &uid, const ParentResource &parent, const String &documentPath, ListCollectionIdsOptions listCollectionIdsOptions, bool async)
    {
        FirestoreOptions options;
        options.requestType = firebase_firestore_request_type_list_collection;
        options.parent = parent;
        options.parent.documentPath = documentPath;
        options.payload = listCollectionIdsOptions.c_str();

        addDocsPath(options.extras);
        URLHelper uh;
        uh.addPath(options.extras, documentPath);
        options.extras += FPSTR(":listCollectionIds");

        async_request_data_t aReq(&aClient, path, async_request_handler_t::http_post, slot_options_t(false, false, async, false, false, false), &options, result, cb, uid);
        asyncRequest(aReq);
    }

    void databaseIndexManager(AsyncClientClass &aClient, AsyncResult *result, AsyncResultCallback cb, const String &uid, const ParentResource &parent, DatabaseIndex::Index index, const String &indexId, bool deleteMode, bool async)
    {
        FirestoreOptions options;
        options.requestType = firebase_firestore_request_type_create_field_index;
        options.parent = parent;
        options.payload = index.c_str();
        options.extras += FPSTR("/indexes");
        if (indexId.length())
            options.extras += '/';
        options.extras += indexId;

        async_request_handler_t::http_request_method method = async_request_handler_t::http_undefined;

        if (strlen(index.c_str()) > 0)
            method = async_request_handler_t::http_post; // create
        else if (indexId.length() > 0)
            method = deleteMode ? async_request_handler_t::http_delete : async_request_handler_t::http_get; // get index or delete by id
        if (strlen(index.c_str()) == 0 && indexId.length() == 0)
            method = async_request_handler_t::http_get; // list

        async_request_data_t aReq(&aClient, path, method, slot_options_t(false, false, async, false, false, false), &options, result, cb, uid);
        asyncRequest(aReq, 1);
    }

    void collectionGroupIndexManager(AsyncClientClass &aClient, AsyncResult *result, AsyncResultCallback cb, const String &uid, const ParentResource &parent, CollectionGroupsIndex::Index index, const String &collectionId, const String &indexId, bool deleteMode, bool async)
    {
        FirestoreOptions options;
        options.requestType = firebase_firestore_request_type_create_composite_index;
        options.parent = parent;
        options.payload = index.c_str();
        options.extras += FPSTR("/collectionGroups");
        if (collectionId.length())
            options.extras += '/';
        options.extras += collectionId;
        options.extras += FPSTR("/indexes");

        if (indexId.length())
        {
            options.extras += '/';
            options.extras += indexId;
        }

        async_request_handler_t::http_request_method method = async_request_handler_t::http_undefined;

        if (strlen(index.c_str()) > 0)
            method = async_request_handler_t::http_post; // create
        else if (collectionId.length() > 0)
            method = deleteMode ? async_request_handler_t::http_delete : async_request_handler_t::http_get; // get index or delete by id
        if (strlen(index.c_str()) == 0 && indexId.length() == 0)
            method = async_request_handler_t::http_get; // list

        async_request_data_t aReq(&aClient, path, method, slot_options_t(false, false, async, false, false, false), &options, result, cb, uid);
        asyncRequest(aReq);
    }

    String makeResourcePath(const ParentResource &parent)
    {
        String str = FPSTR("projects/");
        str += parent.projectId;
        addDatabasePath(str);
        str += '/';
        str += parent.databaseId.length() > 0 ? parent.databaseId : FPSTR("(default)");
        addDocsPath(str);
        return str;
    }
    void addDatabasePath(String &buf) { buf += FPSTR("/databases"); }
    void addDocsPath(String &buf) { buf += FPSTR("/documents"); }
};

#endif

#endif