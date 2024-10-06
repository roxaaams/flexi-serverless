terraform {
  required_providers {
    aws = {
      source  = "hashicorp/aws"
      version = "~> 5.0"
    }
  }
}

provider "aws" {
  region = var.aws_region
}

locals {
  functions = [
    {
      name               = "stringsimilarity"
      handler            = "index.handler"
      runtime            = "nodejs20.x"
      timeout_in_seconds = 5
      memory_size        = 512

    },
    {
      name               = "imagefiltering"
      handler            = "index.handler"
      runtime            = "nodejs20.x"
      timeout_in_seconds = 900
      memory_size        = 9728

    },
    {
      name               = "montecarlo"
      handler            = "index.handler"
      runtime            = "nodejs20.x"
      timeout_in_seconds = 5
      memory_size        = 512

    }
  ]
}

resource "aws_lambda_function" "function" {
  count = length(local.functions)

  function_name = "${local.functions[count.index].name}-${var.env_name}"
  handler       = local.functions[count.index].handler
  runtime       = local.functions[count.index].runtime
  timeout       = local.functions[count.index].timeout_in_seconds
  memory_size   = local.functions[count.index].memory_size

  filename         = "${path.module}/functions/${local.functions[count.index].name}.zip"
  source_code_hash = data.archive_file.function_zip[count.index].output_base64sha256

  role = aws_iam_role.function_role[count.index].arn

  environment {
    variables = {
      ENVIRONMENT = var.env_name
    }
  }
}

data "archive_file" "function_zip" {
  count = length(local.functions)

  source_dir  = "${path.module}/functions/${local.functions[count.index].name}"
  type        = "zip"
  output_path = "${path.module}/functions/${local.functions[count.index].name}.zip"
}

resource "aws_iam_role" "function_role" {
  count = length(local.functions)

  name = "${local.functions[count.index].name}-${var.env_name}"

  assume_role_policy = jsonencode({
    Statement = [
      {
        Action = "sts:AssumeRole"
        Effect = "Allow"
        Principal = {
          Service = "lambda.amazonaws.com"
        }
      },
    ]
  })
}
