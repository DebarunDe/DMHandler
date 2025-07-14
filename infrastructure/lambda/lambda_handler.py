def main(event,context):
    print("Received event: ", event)
    print(json.dumps(event, indent=2))

    try:
        for record in event['Records']:
            s3_info = record['s3']
            bucket_name = s3_info['bucket']['name']
            object_key = s3_info['object']['key']
            
            print(f"New object created in bucket '{bucket_name}' at key '{object_key}'")
            
            # Example: trigger downstream logic here
            # process_new_binary(bucket_name, object_key)
    except Exception as e:
        print(f"Error parsing S3 event: {e}")